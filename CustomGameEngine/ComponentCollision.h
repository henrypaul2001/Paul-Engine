#pragma once
#include "Entity.h"
#include <unordered_map>
#include <glm/ext/matrix_float4x4.hpp>

// The whole physics system and collision setup is a mess anyway. Will be re-worked as a larger physics engine re-design

namespace Engine {

    enum ColliderType {
        COLLISION_SPHERE,
        COLLISION_BOX,
        COLLISION_AABB
    };

    // Bounding box structure and implementation below adapted from: https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/csc8503coderepository/
    struct BoxEdge {
        int id;
        int startVertexId, endVertexId;
        std::vector<int> adjoiningEdgeIds;
        std::vector<int> enclosingFaceIds;
    };

    struct BoxFace {
        int id;
        glm::vec3 normal;
        std::vector<int> vertexIds;
        std::vector<int> edgeIds;
        std::vector<int> adjoiningFaceIds;
    };

    struct BoxVertex {
        BoxVertex() { id = 0; position = glm::vec3(0.0f); }
        BoxVertex(int id, glm::vec3 position) { this->id = id; this->position = position; }

        int id;
        glm::vec3 position;
        std::vector<int> enclosingEdgeIds;
        std::vector<int> enclosingFaceIds;
    };

    struct BoundingBox {
        BoundingBox() {}

        std::vector<BoxVertex> vertices;
        std::vector<BoxEdge> edges;
        std::vector<BoxFace> faces;

        int AddVertex(glm::vec3 position) {
            int size = vertices.size(); // .size() isn't zero indexed so this is fine
            vertices.push_back(BoxVertex(size, position));
            return size;
        }

        int AddFace(glm::vec3 normal, int numVertices, int vertices[]) {
            BoxFace newFace;
            newFace.id = faces.size();
            newFace.normal = glm::normalize(normal);

            // Construct all edges
            int point0 = numVertices - 1;
            for (int point1 = 0; point1 < numVertices; point1++) {
                newFace.vertexIds.push_back(vertices[point1]);
                newFace.edgeIds.push_back(ConstructNewEdge(newFace.id, vertices[point0], vertices[point1]));
                point0 = point1;
            }

            // Find adjacent faces
            for (int i = 0; i < newFace.id; i++) {
                BoxFace& currentFace = faces[i];
                bool found = false;
                for (int j = 0; found == false && j < currentFace.edgeIds.size(); j++) {
                    for (int k = 0; found == false && k < numVertices; k++) {
                        if (newFace.edgeIds[k] == currentFace.edgeIds[j]) {
                            found = true;
                            currentFace.adjoiningFaceIds.push_back(newFace.id);
                            newFace.adjoiningFaceIds.push_back(i);
                        }
                    }
                }
            }

            // Update contained vertices
            for (int i = 0; i < numVertices; i++) {
                BoxVertex& start = this->vertices[edges[newFace.edgeIds[i]].startVertexId];
                BoxVertex& end = this->vertices[edges[newFace.edgeIds[i]].endVertexId];

                auto foundStart = std::find(start.enclosingFaceIds.begin(), start.enclosingFaceIds.end(), newFace.id);
                if (foundStart == start.enclosingFaceIds.end()) {
                    start.enclosingFaceIds.push_back(newFace.id);
                }

                auto foundEnd = std::find(end.enclosingFaceIds.begin(), end.enclosingFaceIds.end(), newFace.id);
                if (foundEnd == end.enclosingFaceIds.end()) {
                    end.enclosingFaceIds.push_back(newFace.id);
                }
            }

            faces.push_back(newFace);
            return newFace.id;
        }

        int FindEdge(int vertexIdA, int vertexIdB) {
            for (const BoxEdge& edge : edges) {
                if ((edge.startVertexId == vertexIdA && edge.endVertexId == vertexIdB) || (edge.startVertexId == vertexIdB && edge.endVertexId == vertexIdA)) {
                    return edge.id;
                }
            }
            return -1; // edge not found
        }

        int ConstructNewEdge(int parentFaceId, int vertexStart, int vertexEnd) {
            int id = FindEdge(vertexStart, vertexEnd);

            if (id == -1) {
                // Edge doesn't yet exist
                id = edges.size();

                BoxEdge newEdge;
                newEdge.id = id;
                newEdge.startVertexId = vertexStart;
                newEdge.endVertexId = vertexEnd;

                // Find adjacent edges
                for (int i = 0; i < newEdge.id; i++) {
                    if (edges[i].startVertexId == vertexStart
                        || edges[i].startVertexId == vertexEnd
                        || edges[i].endVertexId == vertexStart
                        || edges[i].endVertexId == vertexEnd)
                    {
                        edges[i].adjoiningEdgeIds.push_back(newEdge.id);
                        newEdge.adjoiningEdgeIds.push_back(i);
                    }
                }

                // Update contained vertices
                vertices[vertexStart].enclosingEdgeIds.push_back(newEdge.id);
                vertices[vertexEnd].enclosingEdgeIds.push_back(newEdge.id);

                edges.push_back(newEdge);
            }

            edges[id].enclosingFaceIds.push_back(parentFaceId);
            return id;
        }

        void GetMinMaxVerticesOnAxis(const glm::vec3 localAxis, int& out_minIndex, int& out_maxIndex) {
            float correlation;

            float minCorrelation = FLT_MAX, maxCorrelation = -FLT_MAX;

            for (size_t i = 0; i < vertices.size(); ++i)
            {
                correlation = glm::dot(localAxis, vertices[i].position);

                if (correlation > maxCorrelation)
                {
                    maxCorrelation = correlation;
                    out_maxIndex = i;
                }

                if (correlation <= minCorrelation)
                {
                    minCorrelation = correlation;
                    out_minIndex = i;
                }
            }
        }
    };

    struct ClippingPlane {
        ClippingPlane() { normal = glm::vec3(); distance = 0.0f; }
        ClippingPlane(glm::vec3 normal, float distance) { this->normal = normal; this->distance = distance; }

        bool PointInPlane(const glm::vec3& position) const {
            if (glm::dot(position, normal) + distance < 0.0f) {
                return false;
            }

            return true;
        }

        glm::vec3 normal;
        float distance;
    };
	
    class ComponentCollision
	{
	public:
        virtual constexpr ColliderType ColliderType() const = 0;

		void ClearEntitiesCheckedThisFrame() { EntitiesCheckedThisFrame.clear(); }
		void AddToEntitiesCheckedThisFrame(const unsigned int e, const std::string& name);

		bool HasEntityAlreadyBeenChecked(const unsigned int e) const { return EntitiesCheckedThisFrame.find(e) != EntitiesCheckedThisFrame.end(); }

		bool IsMovedByCollisions() const { return isMovedByCollisions; }
		void IsMovedByCollisions(const bool isMoveable) { isMovedByCollisions = isMoveable; }

		const std::unordered_map<unsigned int, std::string>& Collisions() { return EntitiesCollidingWith; }
		bool IsCollidingWithEntity(const unsigned int e) const { return EntitiesCollidingWith.find(e) != EntitiesCollidingWith.end(); }
		void AddToCollisions(const unsigned int e, const std::string& name);
		void RemoveFromCollisions(const unsigned int e) { EntitiesCollidingWith.erase(e); }
	
    protected:
        std::unordered_map<unsigned int, std::string> EntitiesCheckedThisFrame;
        std::unordered_map<unsigned int, std::string> EntitiesCollidingWith;

        bool isMovedByCollisions;
    };
}