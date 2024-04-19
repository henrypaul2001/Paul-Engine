#pragma once
#include "ComponentCollision.h"
#include "ComponentTransform.h"
namespace Engine {
    struct BoxExtents {
        float minX;
        float minY;
        float minZ;

        float maxX;
        float maxY;
        float maxZ;

        float GetBiggestExtent() {
            float x = maxX - minX;
            float y = maxY - minY;
            float z = maxZ - minZ;
            float biggest = x;
            if (y > biggest) { biggest = y; }
            if (z > biggest) { biggest = z; }
            return biggest;
        }
    };

    class ComponentCollisionBox : public ComponentCollision
    {
    public:
        ComponentCollisionBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
        ~ComponentCollisionBox();

        BoxExtents GetLocalPoints() { return localExtents; }
        void SetLocalPoints(BoxExtents newPoints) { localExtents = newPoints; }

        void SetMinX(float minX) { localExtents.minX = minX; }
        void SetMinY(float minY) { localExtents.minY = minY; }
        void SetMinZ(float minZ) { localExtents.minZ = minZ; }

        void SetMaxX(float maxX) { localExtents.maxX = maxX; }
        void SetMaxY(float maxY) { localExtents.maxY = maxY; }
        void SetMaxZ(float maxZ) { localExtents.maxZ = maxZ; }

        bool CheckBroadPhaseFirst() { return checkBroadPhaseFirst; }
        void CheckBroadPhaseFirst(bool checkBroadPhase) { checkBroadPhaseFirst = checkBroadPhase; }

        BoundingBox& GetBoundingBox() { return boundingBox; }

        BoxExtents GetWorldSpacePoints(glm::mat4 modelMatrix);
        std::vector<glm::vec3> WorldSpacePoints(glm::mat4 modelMatrix);

        ComponentTypes ComponentType() override { return COMPONENT_COLLISION_BOX; }
        void Close() override;
    private:
        void ConstructCube();

        BoxExtents localExtents;
        BoundingBox boundingBox;

        bool checkBroadPhaseFirst;
    };
}