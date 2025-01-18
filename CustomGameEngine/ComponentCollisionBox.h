#pragma once
#include "ComponentCollision.h"
namespace Engine {
    struct BoxExtents {
        float minX;
        float minY;
        float minZ;

        float maxX;
        float maxY;
        float maxZ;

        float GetBiggestExtent() const {
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
        constexpr Engine::ColliderType ColliderType() const override { return COLLISION_BOX; }

        ComponentCollisionBox(const float minX, const float minY, const float minZ, const float maxX, const float maxY, const float maxZ);
        ~ComponentCollisionBox();

        const BoxExtents& GetLocalPoints() const { return localExtents; }
        void SetLocalPoints(const BoxExtents& newPoints) { localExtents = newPoints; }

        void SetMinX(const float minX) { localExtents.minX = minX; }
        void SetMinY(const float minY) { localExtents.minY = minY; }
        void SetMinZ(const float minZ) { localExtents.minZ = minZ; }

        void SetMaxX(const float maxX) { localExtents.maxX = maxX; }
        void SetMaxY(const float maxY) { localExtents.maxY = maxY; }
        void SetMaxZ(const float maxZ) { localExtents.maxZ = maxZ; }

        bool CheckBroadPhaseFirst() const { return checkBroadPhaseFirst; }
        void CheckBroadPhaseFirst(const bool checkBroadPhase) { checkBroadPhaseFirst = checkBroadPhase; }

        const BoundingBox& GetBoundingBox() const { return boundingBox; }

        BoxExtents GetWorldSpacePoints(const glm::mat4& modelMatrix);
        std::vector<glm::vec3> WorldSpacePoints(const glm::mat4& modelMatrix);
    private:
        void ConstructCube();

        BoxExtents localExtents;
        BoundingBox boundingBox;

        bool checkBroadPhaseFirst;
    };
}