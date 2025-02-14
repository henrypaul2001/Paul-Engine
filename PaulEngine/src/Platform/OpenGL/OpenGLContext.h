#pragma once
#include "PaulEngine/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace PaulEngine {
    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* window);
        ~OpenGLContext();

        virtual void Init() override;
        virtual void SwapBuffers() override;

    private:
        GLFWwindow* m_Window;
    };
}