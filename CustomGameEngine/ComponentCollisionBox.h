#pragma once
#include "ComponentCollision.h"
#include <glm/ext/matrix_float4x4.hpp>
#include "ComponentTransform.h"
namespace Engine {
    struct BoxPoints {
        float minX;
        float minY;
        float minZ;

        float maxX;
        float maxY;
        float maxZ;
    };

    class ComponentCollisionBox : public ComponentCollision
    {
    public:
        ComponentCollisionBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool defaultCollisionResponse);
        ~ComponentCollisionBox();

        BoxPoints GetLocalPoints() { return localPoints; }
        void SetLocalPoints(BoxPoints newPoints) { localPoints = newPoints; }

        void SetMinX(float minX) { localPoints.minX = minX; }
        void SetMinY(float minY) { localPoints.minY = minY; }
        void SetMinZ(float minZ) { localPoints.minZ = minZ; }

        void SetMaxX(float maxX) { localPoints.maxX = maxX; }
        void SetMaxY(float maxY) { localPoints.maxY = maxY; }
        void SetMaxZ(float maxZ) { localPoints.maxZ = maxZ; }

        BoxPoints GetWorldSpacePoints(glm::mat4 modelMatrix);
        std::vector<glm::vec3> WorldSpacePoints(ComponentTransform* transform);

        ComponentTypes ComponentType() override { return COMPONENT_COLLISION_BOX; }
        void Close() override;
    private:
        BoxPoints localPoints;
    };
}