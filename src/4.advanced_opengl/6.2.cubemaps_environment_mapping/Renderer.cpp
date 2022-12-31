#include "Renderer.h"

#include <iostream>
#include <functional>
#include <numeric>

//#include <learnopengl/filesystem.h>
//#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

void window_mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    static float lastX = INIT_WIDTH / 2.0;
    static float lastY = INIT_HEIGHT / 2.0;
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    Renderer::GetCameraInstance()->ProcessMouseMovement(xoffset, yoffset);
};

void window_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Renderer::GetCameraInstance()->ProcessMouseScroll(static_cast<float>(yoffset));
}; 

Renderer::Renderer()
{
    //m_camera = new Camera();
    //m_camera = std::make_shared<Camera>(new Camera());//裸指针不能赋值 需要make_shared内部调用构造后转成shared_ptr
}

Renderer::~Renderer()
{

}

bool Renderer::initWindows()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    m_glfwWin = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (m_glfwWin == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_glfwWin);

    //无法获取函数指针
    /*std::function<void(GLFWwindow* window, int width, int height)> framebuffer_size_callback{ 
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }
    };*/

    /*void (*framebuffer_size_callback)(GLFWwindow * window, int width, int height) {
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }
    };*/

    //lambda空捕获的时候 可以当函数指针用
    auto framebuffer_size_callback = 
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        };
    
    //lambda表达式捕获值的时候无法当函数指针用
    /*auto scroll_callback =
        [=](GLFWwindow* window, double xoffset, double yoffset)
    {
        this->GetCamera()->ProcessMouseScroll(static_cast<float>(yoffset));
    };*/


    glfwSetFramebufferSizeCallback(m_glfwWin, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_glfwWin, window_mouse_callback);
    glfwSetScrollCallback(m_glfwWin, window_scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(m_glfwWin, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}

bool Renderer::isWindowsClosed()
{
    return glfwWindowShouldClose(m_glfwWin);
}

void Renderer::processWindowsInput()
{
    if (glfwGetKey(m_glfwWin, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_glfwWin, true);

    if (glfwGetKey(m_glfwWin, GLFW_KEY_W) == GLFW_PRESS)
        Renderer::GetCameraInstance()->ProcessKeyboard(FORWARD, m_deltaTime);
    if (glfwGetKey(m_glfwWin, GLFW_KEY_S) == GLFW_PRESS)
        Renderer::GetCameraInstance()->ProcessKeyboard(BACKWARD, m_deltaTime);
    if (glfwGetKey(m_glfwWin, GLFW_KEY_A) == GLFW_PRESS)
        Renderer::GetCameraInstance()->ProcessKeyboard(LEFT, m_deltaTime);
    if (glfwGetKey(m_glfwWin, GLFW_KEY_D) == GLFW_PRESS)
        Renderer::GetCameraInstance()->ProcessKeyboard(RIGHT, m_deltaTime);
}

void Renderer::swapWindowsBuffer()
{
    glfwSwapBuffers(m_glfwWin);
}

void Renderer::pollWindowsEvent()
{
    glfwPollEvents();
}

void Renderer::closeWindow()
{
    glfwTerminate();
}

bool Renderer::initGLFunction()
{
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

void Renderer::createVertexBuffer(uint32_t& vao, uint32_t& vbo, std::vector<float>& vtxData, std::vector<int> vtxAttrSizeSet)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vtxData.size(), vtxData.data(), GL_STATIC_DRAW);

    int vtxAttriCount = vtxAttrSizeSet.size();//一个顶点多少个属性
    int vtxAttriLength = accumulate(vtxAttrSizeSet.begin(), vtxAttrSizeSet.end(), (int)0); //一个顶点的长度
    int offset = 0;
    for (int idx = 0; idx < vtxAttriCount; idx++)
    {
        if (idx > 0)
        {
            offset += vtxAttrSizeSet[idx - 1];
        }
        glEnableVertexAttribArray(idx);
        glVertexAttribPointer(idx, vtxAttrSizeSet[idx], GL_FLOAT, GL_FALSE, vtxAttriLength * sizeof(float), (void*)(offset * sizeof(float)));
    }

}

void Renderer::bindVertexArrayObject(uint32_t vao)
{
    glBindVertexArray(vao);
}

void Renderer::unBindVertexArrayObject()
{
    glBindVertexArray(0);
}

void Renderer::activeTexture(uint32_t texEnum)
{
    glActiveTexture(texEnum);
}

void Renderer::bindTexture(uint32_t texType, uint32_t texId)
{
    glBindTexture(texType, texId);
}

void Renderer::drawTriangle(int vtxCount)
{
    glDrawArrays(GL_TRIANGLES, 0, vtxCount);
}

void Renderer::onTimeUpdate()
{
    float currentTime = static_cast<float>(glfwGetTime());
    m_deltaTime = currentTime - m_lastTime;
    m_lastTime = currentTime;
}

void Renderer::setClearFlag(uint8_t flag, uint32_t rgba, float depth, uint32_t stencil)
{
    uint32_t glFlag = 0x00000000;
    if (flag & 0x01) //有clearcolor标记
    {
        glFlag |= GL_COLOR_BUFFER_BIT;
        float red = (rgba & 0xff000000) >> 6;    red /= 255.0;
        float green = (rgba & 0x00ff0000) >> 4;    green /= 255.0;
        float blue = (rgba & 0x0000ff00) >> 2;    blue /= 255.0;
        float alpha = (rgba & 0x000000ff) >> 0;    alpha /= 255.0;
        glClearColor(red, green, blue, alpha);
    }

    if (flag & 0x02) //有cleardepth标记
    {
        glFlag |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth);
    }

    if (flag & 0x04) //有clearstencil标记
    {
        glFlag |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(stencil);
    }

    if (glFlag > 0)
    {
        glClear(glFlag);
    }
}


void Renderer::createShader(std::string str, std::string vertex, std::string frag)
{
    if (!m_shaderMap[str])
    {
        m_shaderMap[str] = std::make_shared<Shader>(vertex.c_str(), frag.c_str());
    }
}

std::shared_ptr<Shader> Renderer::getShader(std::string str)
{
    return m_shaderMap[str];
}

uint32_t Renderer::loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

uint32_t Renderer::loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

Camera* Renderer::GetCameraInstance()
{
    static Camera camera(glm::vec3(0.0, 0.0, 5.0));
    return &camera;
}