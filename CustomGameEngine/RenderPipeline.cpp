#include "RenderPipeline.h"
#include "SystemRender.h"'
#include "SystemShadowMapping.h"
#include "ResourceManager.h"
namespace Engine {
	RenderPipeline::RenderPipeline()
	{

	}

	RenderPipeline::~RenderPipeline()
	{

	}

	void RenderPipeline::Run(std::vector<System*> renderSystems, std::vector<Entity*> entities)
	{
		this->entities = entities;

		shadowmapSystem = nullptr;
		renderSystem = nullptr;
		renderInstance = RenderManager::GetInstance();

		shadowWidth = renderInstance->ShadowWidth();
		shadowHeight = renderInstance->ShadowHeight();

		screenWidth = renderInstance->ScreenWidth();
		screenHeight = renderInstance->ScreenHeight();

		for (System* s : renderSystems) {
			if (s->Name() == SYSTEM_RENDER) {
				renderSystem = dynamic_cast<SystemRender*>(s);
			}
			else if (s->Name() == SYSTEM_SHADOWMAP) {
				shadowmapSystem = dynamic_cast<SystemShadowMapping*>(s);
			}
		}
	}
}