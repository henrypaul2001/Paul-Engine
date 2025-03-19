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

// Render
#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/RenderCommand.h"
#include "PaulEngine/Renderer/Buffer.h"
#include "PaulEngine/Renderer/VertexArray.h"
#include "PaulEngine/Renderer/Shader.h"
#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/SubTexture2D.h"
#include "PaulEngine/Renderer/OrthographicCamera.h"
#include "PaulEngine/Renderer/Framebuffer.h"
#include "PaulEngine/Renderer/UniformBuffer.h"