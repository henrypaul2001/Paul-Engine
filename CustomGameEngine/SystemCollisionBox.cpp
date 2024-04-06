#include "SystemCollisionBox.h"
namespace Engine {
	SystemCollisionBox::SystemCollisionBox(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionBox::~SystemCollisionBox()
	{

	}

	void SystemCollisionBox::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentCollisionBox* collider = nullptr;
			for (Component* c : components) {
				collider = dynamic_cast<ComponentCollisionBox*>(c);
				if (collider != nullptr) {
					break;
				}
			}

			// Loop through all over entities to find another box collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & MASK) == MASK) {
					// Check if this entity has already checked for collisions with current entity in a previous run during this frame
					if (!collider->HasEntityAlreadyBeenChecked(e) && e != entity) {

						std::vector<Component*> components2 = e->Components();

						ComponentTransform* transform2 = nullptr;
						for (Component* c : components2) {
							transform2 = dynamic_cast<ComponentTransform*>(c);
							if (transform2 != nullptr) {
								break;
							}
						}

						ComponentCollisionBox* collider2 = nullptr;
						for (Component* c : components2) {
							collider2 = dynamic_cast<ComponentCollisionBox*>(c);
							if (collider2 != nullptr) {
								break;
							}
						}

						// Check for collision
						Collision(transform, collider, transform2, collider2);
					}
				}
			}
		}
	}

	void SystemCollisionBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & MASK) == MASK) {
				dynamic_cast<ComponentCollisionBox*>(e->GetComponent(COMPONENT_COLLISION_BOX))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		std::vector<glm::vec3> axes = GetAllCollisionAxis(transform, transform2);

		CollisionData collision;
		CollisionData bestCollision;
		bestCollision.AddContactPoint(glm::vec3(), glm::vec3(), glm::vec3(), -FLT_MAX);
		for (glm::vec3 axis : axes) {
			if (axis != glm::vec3(0.0f, 0.0f, 0.0f)) {
				collision.objectA = transform->GetOwner();
				collision.objectB = transform2->GetOwner();
				if (!CheckForCollisionOnAxis(axis, transform, dynamic_cast<ComponentCollisionBox*>(collider), transform2, dynamic_cast<ComponentCollisionBox*>(collider2), collision)) {
					collision.isColliding = false;
					return collision;
				}

				if (collision.contactPoints[0].penetration >= bestCollision.contactPoints[0].penetration) {
					bestCollision = collision;
				}
				collision = CollisionData();
			}
		}

		bestCollision.isColliding = true;

		GetContactPoints(bestCollision);

		return bestCollision;
	}

	void SystemCollisionBox::GetContactPoints(CollisionData& out_collisionInfo)
	{
		std::vector<glm::vec3> poly1, poly2;
		glm::vec3 normal1, normal2;
		std::vector<ClippingPlane> adjPlanes1, adjPlanes2;

		// Get incident reference polygon 1
		GetIncidentReferencePolygon(out_collisionInfo.contactPoints[0].normal, poly1, normal1, adjPlanes1, out_collisionInfo.objectA);

		// Get incident reference polygon 2
		GetIncidentReferencePolygon(-out_collisionInfo.contactPoints[0].normal, poly2, normal2, adjPlanes2, out_collisionInfo.objectB);

		float penatration = out_collisionInfo.contactPoints[0].penetration;
		glm::vec3 normal = out_collisionInfo.contactPoints[0].normal;
		// return if either polygon contains no contact points
		if (poly1.size() == 0 || poly2.size() == 0) {
			return;
		}
		else if (poly1.size() == 1) {
			out_collisionInfo.contactPoints.clear();
			out_collisionInfo.AddContactPoint(poly1.front(), poly1.front() + normal * penatration, normal, penatration);
		}
		else if (poly2.size() == 1) {
			out_collisionInfo.contactPoints.clear();
			out_collisionInfo.AddContactPoint(poly2.front() - normal * penatration, poly2.front(), normal, penatration);
		}
		else {
			// Clipping method
			// Check if we need to flip the incident and reference faces
			bool flipped = fabs(glm::dot(normal, normal1)) < fabs(glm::dot(normal, normal2));
			if (flipped) {
				std::swap(poly1, poly2);
				std::swap(normal1, normal2);
				std::swap(adjPlanes1, adjPlanes2);
			}

			// Clip incident face to adjacent edges of reference face
			if (adjPlanes1.size() > 0) {
				SutherlandHodgmanClipping(poly2, adjPlanes1.size(), &adjPlanes1[0], &poly2, false);
			}

			// Clip and remove any contact points that are above the reference face
			ClippingPlane refPlane = ClippingPlane(-normal1, -glm::dot(-normal1, poly1.front()));
			SutherlandHodgmanClipping(poly2, 1, &refPlane, &poly2, true);

			// Now left with selection of valid contact points to be used for collision manifold
			bool first = true;
			for (const glm::vec3& point : poly2) {
				// Get distance to reference plane
				glm::vec3 pointDiff = point - GetClosestPointPolygon(point, poly1);
				float contact_penetration = glm::dot(pointDiff, normal);

				// set contact data
				glm::vec3 globalOnA = point;
				glm::vec3 globalOnB = point - normal * contact_penetration;

				if (flipped) {
					contact_penetration = -contact_penetration;
					globalOnA = point + normal * contact_penetration;
					globalOnB = point;
				}

				if (contact_penetration < 0.0f) {
					if (first) {
						out_collisionInfo.contactPoints.clear();
						first = false;
					}
					glm::vec3 localA = globalOnA - dynamic_cast<ComponentTransform*>(out_collisionInfo.objectA->GetComponent(COMPONENT_TRANSFORM))->GetWorldPosition();
					glm::vec3 localB = globalOnB - dynamic_cast<ComponentTransform*>(out_collisionInfo.objectB->GetComponent(COMPONENT_TRANSFORM))->GetWorldPosition();
					//glm::vec3 newNormal = glm::normalize(-normal1 + normal2);
					out_collisionInfo.AddContactPoint(localA, localB, normal, contact_penetration);
				}
			}
		}
	}

	void SystemCollisionBox::GetIncidentReferencePolygon(const glm::vec3& axis, std::vector<glm::vec3>& out_face, glm::vec3& out_normal, std::vector<ClippingPlane>& out_adjPlanes, Entity* object)
	{
		glm::mat4 modelMatrix = dynamic_cast<ComponentTransform*>(object->GetComponent(COMPONENT_TRANSFORM))->GetWorldModelMatrix();
		glm::mat3 inverseNormalMatrix = glm::inverse(glm::mat3(modelMatrix));
		glm::mat3 normalMatrix = glm::inverse(inverseNormalMatrix);

		glm::vec3 localAxis = inverseNormalMatrix * axis;

		// Get furthest vertex along axis - furthest face
		int minVertexId, maxVertexId;
		BoundingBox& cube = dynamic_cast<ComponentCollisionBox*>(object->GetComponent(COMPONENT_COLLISION_BOX))->GetBoundingBox();
		cube.GetMinMaxVerticesOnAxis(localAxis, minVertexId, maxVertexId);
		BoxVertex& vertex = cube.vertices[maxVertexId];

		// Get face which is furthest along axis (contains the furthest vertex)
		// Determined by normal being closest to parallel with axis
		const BoxFace* bestFace = nullptr;
		float correlation = -FLT_MAX;
		for (int faceId : vertex.enclosingFaceIds) {
			const BoxFace* face = &cube.faces[faceId];
			float tempCorrelation = glm::dot(localAxis, face->normal);
			if (tempCorrelation > correlation) {
				correlation = tempCorrelation;
				bestFace = face;
			}
		}

		if (bestFace == nullptr) {
			return;
		}

		// Output face normal
		out_normal = glm::normalize((normalMatrix * bestFace->normal));

		// Output face vertices in world space
		for (int vertexId : bestFace->vertexIds) {
			const BoxVertex* vert = &cube.vertices[vertexId];
			out_face.push_back(modelMatrix * glm::vec4(vert->position, 1.0f));
		}

		// Define st of planes that will clip geometry down to fit in the shape. Form list of clip
		// planes from each adjacent face and the reference of the face itself
		glm::vec3 worldPointOnPlane = modelMatrix * glm::vec4(cube.vertices[cube.edges[bestFace->edgeIds[0]].startVertexId].position, 1.0f);

		// Form plane around reference face
		glm::vec3 planeNormal = glm::normalize(-(normalMatrix * bestFace->normal));
		float planeDistance = -glm::dot(planeNormal, worldPointOnPlane);

		// Loop over all adjacent faces and form clip plane
		for (int edgeId : bestFace->edgeIds) {
			const BoxEdge& edge = cube.edges[edgeId];
			worldPointOnPlane = glm::vec3(modelMatrix * glm::vec4(cube.vertices[edge.startVertexId].position, 1.0f));

			for (int adjFaceId : edge.enclosingFaceIds) {
				if (adjFaceId != bestFace->id) {
					const BoxFace& adjFace = cube.faces[adjFaceId];

					glm::vec3 planeNrml = glm::normalize(-(normalMatrix * adjFace.normal));
					float planeDist = -glm::dot(planeNrml, worldPointOnPlane);
					out_adjPlanes.push_back(ClippingPlane(planeNrml, planeDist));
				}
			}
		}
	}
}