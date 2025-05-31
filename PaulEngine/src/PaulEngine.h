#pragma once

// For use by applications only. Not for use in PaulEngine source

#include "pepch.h"
#include "PaulEngine/Core/Application.h"
#include "PaulEngine/Core/Log.h"

#include "PaulEngine/Core/Timestep.h"

// Input
#include "PaulEngine/Core/Input.h"
#include "PaulEngine/Core/KeyCodes.h"
#include "PaulEngine/Core/MouseButtonCodes.h"

// Layer
#include "PaulEngine/ImGui/ImGuiLayer.h"

// Scene
#include "PaulEngine/Scene/Scene.h"
#include "PaulEngine/Scene/Entity.h"
#include "PaulEngine/Scene/EntityScript.h"
#include "PaulEngine/Scene/Components.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/AssetManager.h"

// Render
#include "PaulEngine/Renderer/FrameRenderer.h"
#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Renderer/RenderCommand.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"
#include "PaulEngine/Renderer/Resource/VertexArray.h"
#include "PaulEngine/Renderer/Asset/Shader.h"
#include "PaulEngine/Renderer/Asset/Texture.h"
#include "PaulEngine/Renderer/Asset/SubTexture2D.h"
#include "PaulEngine/Renderer/Resource/OrthographicCamera.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"
#include "PaulEngine/Renderer/Resource/UniformBuffer.h"
#include "PaulEngine/Renderer/Resource/UniformBufferStorage.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/AssetImporter.h"
#include "PaulEngine/Asset/TextureImporter.h"

// Event
#include "PaulEngine/Events/Event.h"
#include "PaulEngine/Events/ApplicationEvent.h"
#include "PaulEngine/Events/KeyEvent.h"
#include "PaulEngine/Events/MouseEvent.h"
#include "PaulEngine/Events/SceneEvent.h"