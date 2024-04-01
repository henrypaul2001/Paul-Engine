#include "SystemCollision.h"
#include "ComponentTransform.h"
namespace Engine {
	SystemCollision::SystemCollision(EntityManager* entityManager, CollisionManager* collisionManager)
	{
		this->entityManager = entityManager;
		this->collisionManager = collisionManager;
	}

	SystemCollision::~SystemCollision()
	{

	}

	void SystemCollision::Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		collider->AddToEntitiesCheckedThisFrame(collider2->GetOwner());
		collider2->AddToEntitiesCheckedThisFrame(collider->GetOwner());

		if (collider->useDefaultCollisionResponse && collider2->useDefaultCollisionResponse) {
			CollisionData collision = Intersect(transform, collider, transform2, collider2);
			if (collision.isColliding) {
				//DefaultCollisionResponse(transform->GetOwner(), transform2->GetOwner());
				collisionManager->AddToCollisionList(collision);

				collider->AddToCollisions(collider2->GetOwner());
				collider2->AddToCollisions(collider->GetOwner());
			}
			else {
				collider->RemoveFromCollisions(collider2->GetOwner());
				collider2->RemoveFromCollisions(collider->GetOwner());
			}
		}
	}

	void SystemCollision::DefaultCollisionResponse(Entity* entity1, Entity* entity2)
	{
		ComponentTransform* transform1 = dynamic_cast<ComponentTransform*>(entity1->GetComponent(COMPONENT_TRANSFORM));
		transform1->SetPosition(transform1->LastPosition());

		ComponentTransform* transform2 = dynamic_cast<ComponentTransform*>(entity2->GetComponent(COMPONENT_TRANSFORM));
		transform2->SetPosition(transform2->LastPosition());
	}

	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max)
	{
		if (worldSpacePoints.size() < 1) {
			return;
		}

		out_min = glm::dot(worldSpacePoints[0], worldSpaceAxis);
		out_max = out_min;

		float projectedPosition;
		for (unsigned int i = 1; i < worldSpacePoints.size(); i++) {
			projectedPosition = glm::dot(worldSpacePoints[i], worldSpaceAxis);
			if (projectedPosition > out_max) {
				out_max = projectedPosition;
			}
			else if (projectedPosition < out_min) {
				out_min = projectedPosition;
			}
		}
	}

	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max, int& out_minIndex, int& out_maxIndex)
	{
		if (worldSpacePoints.size() < 1) {
			return;
		}

		out_min = glm::dot(worldSpacePoints[0], worldSpaceAxis);
		out_minIndex = 0;
		out_max = out_min;
		out_maxIndex = out_minIndex;

		float projectedPosition;
		for (unsigned int i = 1; i < worldSpacePoints.size(); i++) {
			projectedPosition = glm::dot(worldSpacePoints[i], worldSpaceAxis);
			if (projectedPosition > out_max) {
				out_max = projectedPosition;
				out_maxIndex = i;
			}
			else if (projectedPosition < out_min) {
				out_min = projectedPosition;
				out_minIndex = i;
			}
		}
	}

	bool SystemCollision::CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionBox* collider2, CollisionData& collision)
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		std::vector<glm::vec3> cube1 = collider->WorldSpacePoints(transform->GetWorldModelMatrix());
		std::vector<glm::vec3> cube2 = collider2->WorldSpacePoints(transform2->GetWorldModelMatrix());

		// Project points onto axis and check for overlap
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			glm::vec3 collisionNormal = glm::normalize(axis);
			float collisionPenetration = cube2Min - cube1Max;
			glm::vec3 otherLocalCollisionPoint = cube1Max + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			glm::vec3 collisionNormal = -glm::normalize(axis);
			float collisionPenetration = cube1Min - cube2Max;
			glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		return false;
	}

	bool SystemCollision::CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionAABB* collider2, CollisionData& collision)
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		std::vector<glm::vec3> cube1 = collider->WorldSpacePoints(transform->GetWorldModelMatrix());
		std::vector<glm::vec3> cube2 = collider2->WorldSpacePoints(transform2->GetWorldModelMatrix());

		// Project points onto axis and check for overlap
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			glm::vec3 collisionNormal = glm::normalize(axis);
			float collisionPenetration = cube2Min - cube1Max;
			glm::vec3 otherLocalCollisionPoint = cube2Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			glm::vec3 collisionNormal = -glm::normalize(axis);
			float collisionPenetration = cube1Min - cube2Max;
			glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;
			
			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		return false;
	}

	std::vector<glm::vec3> SystemCollision::GetCubeNormals(ComponentTransform* transform)
	{
		glm::mat3 rotationMatrix = glm::mat3(transform->GetWorldModelMatrix());

		std::vector<glm::vec3> normals;
		normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); // right face
		normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // top face
		normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // front face

		// Rotate each normal based on objects current rotation
		for (glm::vec3& normal : normals) {
			normal = glm::normalize(rotationMatrix * normal);
		}

		return normals;
	}

	std::vector<glm::vec3> SystemCollision::GetEdgeVectors(ComponentTransform* transform)
	{
		glm::mat3 rotationMatrix = glm::mat3(transform->GetWorldModelMatrix());

		std::vector<glm::vec3> edges;
		edges.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, 1.0f, -1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, -1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, -1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
		edges.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, 1.0f));

		// Rotate each edge by objects current rotation
		for (glm::vec3& edge : edges) {
			edge = rotationMatrix * edge;
		}

		return edges;
	}

	std::vector<glm::vec3> SystemCollision::GetAllCollisionAxis(ComponentTransform* transform, ComponentTransform* transform2)
	{
		std::vector<glm::vec3> axes;

		// Get normals of both cubes
		std::vector<glm::vec3> cube1Normals = GetCubeNormals(transform);
		std::vector<glm::vec3> cube2Normals = GetCubeNormals(transform2);

		// Get edge vectors of both cubes
		//std::vector<glm::vec3> cube1Edges = GetEdgeVectors(transform);
		//std::vector<glm::vec3> cube2Edges = GetEdgeVectors(transform2);

		// Combine into one list of potential collision axes
		axes = cube1Normals;
		axes.insert(axes.end(), cube2Normals.begin(), cube2Normals.end());

		/*
		// Add cross product of all edges to axes
		for (glm::vec3 edge1 : cube1Edges) {
			for (glm::vec3 edge2 : cube2Edges) {
				axes.push_back(glm::cross(edge1, edge2));
			}
		}
		*/

		// Add cross product of all normals to axes
		for (glm::vec3 normal1 : cube1Normals) {
			for (glm::vec3 normal2 : cube2Normals) {
				axes.push_back(glm::cross(normal1, normal2));
			}
		}

		int size = axes.size();
		return axes;
	}

	// Implementation below adapted from: https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/csc8503coderepository/
	void SystemCollision::SutherlandHodgmanClipping(const std::vector<glm::vec3>& input_polygon, int num_clip_planes, const ClippingPlane* clip_planes, std::vector<glm::vec3>* out_polygon, bool removeNotClipToPlane)
	{
		if (!out_polygon) {
			return;
		}
		if (num_clip_planes == 0) {
			*out_polygon = input_polygon;
		}

		// Create temporary vertices
		std::vector<glm::vec3> ppPolygon1, ppPolygon2;
		std::vector<glm::vec3>* input = &ppPolygon1, *output = &ppPolygon2;

		*input = input_polygon;

		// Iterate over each clipping plane
		for (int i = 0; i < num_clip_planes; i++) {
			// If every point on shape has already been removed, exit
			if (input->empty()) {
				break;
			}

			const ClippingPlane& plane = clip_planes[i];

			// Loop through each edge of the polygon and clip edge against current plane
			glm::vec3 tempPoint, startPoint = input->back();
			for (const glm::vec3& endPoint : *input) {
				bool startInPlane = plane.PointInPlane(startPoint);
				bool endInPlane = plane.PointInPlane(endPoint);

				// If in final pass, remove all points outside reference plane
				if (removeNotClipToPlane) {
					if (endInPlane) {
						output->push_back(endPoint);
					}
				}
				else {
					// If edge is entirely within clipping plane, keep as is
					if (startInPlane && endInPlane) {
						output->push_back(endPoint);
					}
					// If the dge intersects the clipping plane, cut the edge along the clip plane
					else if (startInPlane && !endInPlane) {
						if (PlaneEdgeIntersection(plane, startPoint, endPoint, tempPoint)) {
							output->push_back(tempPoint);
						}
					}
					else if (!startInPlane && endInPlane) {
						if (PlaneEdgeIntersection(plane, startPoint, endPoint, tempPoint)) {
							output->push_back(tempPoint);
						}

						output->push_back(endPoint);
					}
				}
				
				startPoint = endPoint;
			}

			// Swap input / output polygons and clear list
			std::swap(input, output);
			output->clear();
		}

		*out_polygon = *input;
	}

	bool SystemCollision::PlaneEdgeIntersection(const ClippingPlane& plane, const glm::vec3& start, const glm::vec3& end, glm::vec3& out_point)
	{
		glm::vec3 ab = end - start;

		// Check edge and plane are not parallel
		float ab_p = glm::dot(plane.normal, ab);
		if (fabs(ab_p) > 1e-6f) {
			// Generate random point on plane
			glm::vec3 p_co = plane.normal * -plane.distance;

			// Work out edge factor to scale edge by
			float fac = -glm::dot(plane.normal, start - p_co) / ab_p;

			// Stop and large floating point divide issues with close to parallel lines
			fac = std::min(std::max(fac, 0.0f), 1.0f);

			// Return point on edge
			out_point = start + ab * fac;
			return true;
		}

		return false;
	}

	glm::vec3 SystemCollision::GetClosestPointPolygon(const glm::vec3& pos, const std::vector<glm::vec3>& polygon)
	{
		glm::vec3 final_closest_point = glm::vec3(0.0f);
		float final_closest_distsq = FLT_MAX;

		glm::vec3 last = polygon.back();
		for (const glm::vec3& next : polygon) {
			Edge edge = Edge(last, next);
			glm::vec3 edge_closest_point = GetClosestPoint(pos, edge);

			// Compute distance
			glm::vec3 diff = pos - edge_closest_point;
			float temp_distsq = glm::dot(diff, diff);

			if (temp_distsq < final_closest_distsq) {
				final_closest_distsq = temp_distsq;
				final_closest_point = edge_closest_point;
			}

			last = next;
		}

		return final_closest_point;
	}

	glm::vec3 SystemCollision::GetClosestPoint(const glm::vec3& pos, std::vector<Edge>& edges)
	{
		glm::vec3 final_closest_point = glm::vec3(0.0f);
		float final_closest_distsq = FLT_MAX;

		for (Edge& edge : edges) {
			glm::vec3 edge_closest_point = GetClosestPoint(pos, edge);
			
			glm::vec3 diff = pos - edge_closest_point;
			float temp_distsq = glm::dot(diff, diff);

			if (temp_distsq < final_closest_distsq) {
				final_closest_distsq = temp_distsq;
				final_closest_point = edge_closest_point;
			}
		}

		return final_closest_point;
	}

	glm::vec3 SystemCollision::GetClosestPoint(const glm::vec3& pos, Edge& edge)
	{
		glm::vec3 diffAP = pos - edge.start;
		glm::vec3 diffAB = edge.end - edge.start;

		// Distance along line in world space
		float ABAPproduct = glm::dot(diffAP, diffAB);
		float magnitudeAB = glm::dot(diffAB, diffAB);

		float distance = ABAPproduct / magnitudeAB;
		distance = std::max(std::min(distance, 1.0f), 0.0f);

		return edge.start + diffAB * distance;
	}
}