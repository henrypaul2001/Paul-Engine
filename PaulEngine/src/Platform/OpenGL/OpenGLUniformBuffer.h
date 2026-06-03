#pragma once
#include "PaulEngine/Renderer/Resource/UniformBuffer.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"

namespace PaulEngine
{
    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        // default usage = GL_DYNAMIC_DRAW
        OpenGLUniformBuffer(uint32_t size, uint32_t binding, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

        virtual void Bind(uint32_t binding) override;

    private:
        uint32_t m_RendererID;
        BufferUsage m_Usage;
    };
}