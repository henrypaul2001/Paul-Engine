#include "SystemCollision.h"
#include "ComponentTransform.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max) const
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

	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max, int& out_minIndex, int& out_maxIndex) const
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

	bool SystemCollision::CheckForCollisionOnAxis(const glm::vec3& axis, const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionBox& collider2, CollisionData& collision) const
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		const std::vector<glm::vec3> cube1 = collider.WorldSpacePoints(transform.GetWorldModelMatrix());
		const std::vector<glm::vec3> cube2 = collider2.WorldSpacePoints(transform2.GetWorldModelMatrix());

		// Project points onto axis and check for overlap
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			const glm::vec3 collisionNormal = glm::normalize(axis);
			const float collisionPenetration = cube2Min - cube1Max;
			const glm::vec3 otherLocalCollisionPoint = cube1Max + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			const glm::vec3 collisionNormal = -glm::normalize(axis);
			const float collisionPenetration = cube1Min - cube2Max;
			const glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		return false;
	}

	bool SystemCollision::CheckForCollisionOnAxis(const glm::vec3& axis, const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2, CollisionData& collision) const
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		const std::vector<glm::vec3> cube1 = collider.WorldSpacePoints(transform.GetWorldModelMatrix());
		const std::vector<glm::vec3> cube2 = collider2.WorldSpacePoints(transform2.GetWorldModelMatrix());

		// Project points onto axis and check for overlap
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			const glm::vec3 collisionNormal = glm::normalize(axis);
			const float collisionPenetration = cube2Min - cube1Max;
			const glm::vec3 otherLocalCollisionPoint = cube2Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			const glm::vec3 collisionNormal = -glm::normalize(axis);
			const float collisionPenetration = cube1Min - cube2Max;
			const glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;
			
			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		return false;
	}

	std::vector<glm::vec3> SystemCollision::GetCubeNormals(const ComponentTransform& transform) const
	{
		const glm::mat3 rotationMatrix = glm::mat3(transform.GetWorldModelMatrix());

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

	std::vector<glm::vec3> SystemCollision::GetEdgeVectors(const ComponentTransform& transform) const
	{
		glm::mat3 rotationMatrix = glm::mat3(transform.GetWorldModelMatrix());

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

	std::vector<glm::vec3> SystemCollision::GetAllCollisionAxis(const ComponentTransform& transform, const ComponentTransform& transform2) const
	{
		std::vector<glm::vec3> axes;

		// Get normals of both cubes
		const std::vector<glm::vec3> cube1Normals = GetCubeNormals(transform);
		const std::vector<glm::vec3> cube2Normals = GetCubeNormals(transform2);

		// Get edge vectors of both cubes
		//std::vector<glm::vec3> cube1Edges = GetEdgeVectors(transform);
		//std::vector<glm::vec3> cube2Edges = GetEdgeVectors(transform2);

		// Combine into one list of potential collision axes
		axes = cube1Normals;
		for (int i = 0; i < cube1Normals.size(); i++) {
			if (cube2Normals[i] != cube1Normals[i]) {
				axes.push_back(cube2Normals[i]);
			}
		}
		//axes.insert(axes.end(), cube2Normals.begin(), cube2Normals.end());

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
				if (normal1 != normal2) {
					axes.push_back(glm::cross(normal1, normal2));
				}
			}
		}

		int size = axes.size();
		return axes;
	}

	// Implementation below adapted from: https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/csc8503coderepository/
	void SystemCollision::SutherlandHodgmanClipping(const std::vector<glm::vec3>& input_polygon, int num_clip_planes, const ClippingPlane* clip_planes, std::vector<glm::vec3>* out_polygon, const bool removeNotClipToPlane) const
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
				const bool startInPlane = plane.PointInPlane(startPoint);
				const bool endInPlane = plane.PointInPlane(endPoint);

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

	bool SystemCollision::PlaneEdgeIntersection(const ClippingPlane& plane, const glm::vec3& start, const glm::vec3& end, glm::vec3& out_point) const
	{
		const glm::vec3 ab = end - start;

		// Check edge and plane are not parallel
		const float ab_p = glm::dot(plane.normal, ab);
		if (fabs(ab_p) > 1e-6f) {
			// Generate random point on plane
			const glm::vec3 p_co = plane.normal * -plane.distance;

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

	glm::vec3 SystemCollision::GetClosestPointPolygon(const glm::vec3& pos, const std::vector<glm::vec3>& polygon) const
	{
		glm::vec3 final_closest_point = glm::vec3(0.0f);
		float final_closest_distsq = FLT_MAX;

		glm::vec3 last = polygon.back();
		for (const glm::vec3& next : polygon) {
			Edge edge = Edge(last, next);
			const glm::vec3 edge_closest_point = GetClosestPoint(pos, edge);

			// Compute distance
			const glm::vec3 diff = pos - edge_closest_point;
			const float temp_distsq = glm::dot(diff, diff);

			if (temp_distsq < final_closest_distsq) {
				final_closest_distsq = temp_distsq;
				final_closest_point = edge_closest_point;
			}

			last = next;
		}

		return final_closest_point;
	}

	glm::vec3 SystemCollision::GetClosestPoint(const glm::vec3& pos, std::vector<Edge>& edges) const
	{
		glm::vec3 final_closest_point = glm::vec3(0.0f);
		float final_closest_distsq = FLT_MAX;

		for (Edge& edge : edges) {
			const glm::vec3 edge_closest_point = GetClosestPoint(pos, edge);
			
			const glm::vec3 diff = pos - edge_closest_point;
			const float temp_distsq = glm::dot(diff, diff);

			if (temp_distsq < final_closest_distsq) {
				final_closest_distsq = temp_distsq;
				final_closest_point = edge_closest_point;
			}
		}

		return final_closest_point;
	}

	glm::vec3 SystemCollision::GetClosestPoint(const glm::vec3& pos, Edge& edge) const
	{
		const glm::vec3 diffAP = pos - edge.start;
		const glm::vec3 diffAB = edge.end - edge.start;

		// Distance along line in world space
		const float ABAPproduct = glm::dot(diffAP, diffAB);
		const float magnitudeAB = glm::dot(diffAB, diffAB);

		float distance = ABAPproduct / magnitudeAB;
		distance = std::max(std::min(distance, 1.0f), 0.0f);

		return edge.start + diffAB * distance;
	}

	// Use a quick sphere-sphere collision test to see if objects are close enough to check with more complex SAT collision detection
	bool SystemCollision::BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionBox& collider2) const
	{
		const float aRadius = (collider.GetLocalPoints().GetBiggestExtent() / 2.0f)* transform.GetBiggestScaleFactor();
		const float bRadius = (collider2.GetLocalPoints().GetBiggestExtent() / 2.0f) * transform2.GetBiggestScaleFactor();

		const float distanceSqr = glm::distance2(transform.GetWorldPosition(), transform2.GetWorldPosition());

		const float combinedRadius = aRadius + bRadius;
		const float combinedRadiusSqr = combinedRadius * combinedRadius;

		return (distanceSqr < combinedRadiusSqr) ? true : false;
	}
	// Use a quick sphere-sphere collision test to see if objects are close enough to check with more complex SAT collision detection
	bool SystemCollision::BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2) const
	{
		const float aRadius = (collider.GetLocalPoints().GetBiggestExtent() / 2.0f) * transform.GetBiggestScaleFactor();
		const float bRadius = (collider2.GetBoundary().GetBiggestExtent() / 2.0f) * transform2.GetBiggestScaleFactor();

		const float distanceSqr = glm::distance2(transform.GetWorldPosition(), transform2.GetWorldPosition());

		const float combinedRadius = aRadius + bRadius;
		const float combinedRadiusSqr = combinedRadius * combinedRadius;

		return (distanceSqr < combinedRadiusSqr) ? true : false;
	}
	// Use a quick sphere-sphere collision test to see if objects are close enough to check with more complex SAT collision detection
	bool SystemCollision::BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionAABB& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2) const
	{
		const float aRadius = (collider.GetBoundary().GetBiggestExtent() / 2.0f) * transform.GetBiggestScaleFactor();
		const float bRadius = (collider2.GetBoundary().GetBiggestExtent() / 2.0f) * transform2.GetBiggestScaleFactor();

		const float distanceSqr = glm::distance2(transform.GetWorldPosition(), transform2.GetWorldPosition());

		const float combinedRadius = aRadius + bRadius;
		const float combinedRadiusSqr = combinedRadius * combinedRadius;

		return (distanceSqr < combinedRadiusSqr) ? true : false;
	}

	void SystemCollision::GetContactPoints(CollisionData& out_collisionInfo) const
	{
		std::vector<glm::vec3> poly1, poly2;
		glm::vec3 normal1, normal2;
		std::vector<ClippingPlane> adjPlanes1, adjPlanes2;

		// Get incident reference polygon 1
		GetIncidentReferencePolygon(out_collisionInfo.contactPoints[0].normal, poly1, normal1, adjPlanes1, out_collisionInfo.entityIDA);

		// Get incident reference polygon 2
		GetIncidentReferencePolygon(-out_collisionInfo.contactPoints[0].normal, poly2, normal2, adjPlanes2, out_collisionInfo.entityIDB);

		const float penatration = out_collisionInfo.contactPoints[0].penetration;
		const glm::vec3 normal = out_collisionInfo.contactPoints[0].normal;
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
			const bool flipped = fabs(glm::dot(normal, normal1)) < fabs(glm::dot(normal, normal2));
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
			const ClippingPlane refPlane = ClippingPlane(-normal1, -glm::dot(-normal1, poly1.front()));
			SutherlandHodgmanClipping(poly2, 1, &refPlane, &poly2, true);

			// Now left with selection of valid contact points to be used for collision manifold
			bool first = true;
			for (const glm::vec3& point : poly2) {
				// Get distance to reference plane
				const glm::vec3 pointDiff = point - GetClosestPointPolygon(point, poly1);
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
					const glm::vec3 localA = globalOnA - active_ecs->GetComponent<ComponentTransform>(out_collisionInfo.entityIDA)->GetWorldPosition();
					const glm::vec3 localB = globalOnB - active_ecs->GetComponent<ComponentTransform>(out_collisionInfo.entityIDB)->GetWorldPosition();
					//glm::vec3 newNormal = glm::normalize(-normal1 + normal2);
					out_collisionInfo.AddContactPoint(localA, localB, normal, contact_penetration);
				}
			}
		}
	}

	void SystemCollision::GetIncidentReferencePolygon(const glm::vec3& axis, std::vector<glm::vec3>& out_face, glm::vec3& out_normal, std::vector<ClippingPlane>& out_adjPlanes, const unsigned int entityID) const
	{
		const ComponentTransform* transform = active_ecs->GetComponent<ComponentTransform>(entityID);
		const glm::mat4& modelMatrix = transform->GetWorldModelMatrix();
		const glm::mat3 inverseNormalMatrix = glm::inverse(glm::mat3(modelMatrix));
		const glm::mat3 normalMatrix = glm::inverse(inverseNormalMatrix);

		const glm::vec3 localAxis = inverseNormalMatrix * axis;

		// Get furthest vertex along axis - furthest face
		int minVertexId, maxVertexId;
		BoundingBox cube;
		const ComponentCollisionBox* boxCollider = active_ecs->GetComponent<ComponentCollisionBox>(entityID);
		const ComponentCollisionAABB* aabbCollider = active_ecs->GetComponent<ComponentCollisionAABB>(entityID);
		if (boxCollider) { cube = boxCollider->GetBoundingBox(); }
		else if (aabbCollider) { cube = aabbCollider->GetBoundingBox(); }
		else { throw std::invalid_argument("Incident refernc polygon can only be retrieved on objects with either an AABB collider or box collider"); }

		cube.GetMinMaxVerticesOnAxis(localAxis, minVertexId, maxVertexId);
		const BoxVertex& vertex = cube.vertices[maxVertexId];

		// Get face which is furthest along axis (contains the furthest vertex)
		// Determined by normal being closest to parallel with axis
		const BoxFace* bestFace = nullptr;
		float correlation = -FLT_MAX;
		for (int faceId : vertex.enclosingFaceIds) {
			const BoxFace* face = &cube.faces[faceId];
			const float tempCorrelation = glm::dot(localAxis, face->normal);
			if (tempCorrelation > correlation) {
				correlation = tempCorrelation;
				bestFace = face;
			}
		}

		if (bestFace == nullptr) { return; }

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
		const glm::vec3 planeNormal = glm::normalize(-(normalMatrix * bestFace->normal));
		const float planeDistance = -glm::dot(planeNormal, worldPointOnPlane);

		// Loop over all adjacent faces and form clip plane
		for (int edgeId : bestFace->edgeIds) {
			const BoxEdge& edge = cube.edges[edgeId];
			worldPointOnPlane = glm::vec3(modelMatrix * glm::vec4(cube.vertices[edge.startVertexId].position, 1.0f));

			for (int adjFaceId : edge.enclosingFaceIds) {
				if (adjFaceId != bestFace->id) {
					const BoxFace& adjFace = cube.faces[adjFaceId];

					const glm::vec3 planeNormal = glm::normalize(-(normalMatrix * adjFace.normal));
					const float planeDist = -glm::dot(planeNormal, worldPointOnPlane);
					out_adjPlanes.push_back(ClippingPlane(planeNormal, planeDist));
				}
			}
		}
	}
}