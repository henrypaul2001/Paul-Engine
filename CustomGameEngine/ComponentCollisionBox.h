#pragma once
#include "ComponentCollision.h"
#include <glm/ext/matrix_float4x4.hpp>
#include "ComponentTransform.h"
namespace Engine {
    struct BoxExtents {
        float minX;
        float minY;
        float minZ;

        float maxX;
        float maxY;
        float maxZ;
    };

    struct BoundingBox {
        std::vector<BoxVertex> vertices;
        std::vector<BoxEdge> edges;
        std::vector<BoxFace> faces;

        int AddVertex(glm::vec3 position) {

        }

        int AddFace(glm::vec3 normal, int numVertices, int vertices[]) {

        }

        int FindEdge(int vertexIdA, int vertexIdB) {

        }

        int ConstructNewEdge(int parentFaceId, int vertexStart, int vertexEnd) {

        }
    };
    
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
        BoxVertex() { id = 0; position = glm::vec3(0.0f) }
        BoxVertex(int id, glm::vec3 position) { this->id = id; this->position = position; }

        int id;
        glm::vec3 position;
        std::vector<int> enclosingEdgeIds;
        std::vector<int> enclosingFaceIds;
    };

    class ComponentCollisionBox : public ComponentCollision
    {
    public:
        ComponentCollisionBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool defaultCollisionResponse);
        ~ComponentCollisionBox();

        BoxExtents GetLocalPoints() { return localExtents; }
        void SetLocalPoints(BoxExtents newPoints) { localExtents = newPoints; }

        void SetMinX(float minX) { localExtents.minX = minX; }
        void SetMinY(float minY) { localExtents.minY = minY; }
        void SetMinZ(float minZ) { localExtents.minZ = minZ; }

        void SetMaxX(float maxX) { localExtents.maxX = maxX; }
        void SetMaxY(float maxY) { localExtents.maxY = maxY; }
        void SetMaxZ(float maxZ) { localExtents.maxZ = maxZ; }

        BoundingBox& GetBoundingBox() { return boundingBox; }

        BoxExtents GetWorldSpacePoints(glm::mat4 modelMatrix);
        std::vector<glm::vec3> WorldSpacePoints(glm::mat4 modelMatrix);

        ComponentTypes ComponentType() override { return COMPONENT_COLLISION_BOX; }
        void Close() override;
    private:
        void ConstructCube();

        BoxExtents localExtents;
        BoundingBox boundingBox;
    };
}