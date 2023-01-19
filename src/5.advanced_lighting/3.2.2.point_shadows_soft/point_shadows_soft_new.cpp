#if 1

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <Renderer/Renderer.h>

#include <iostream>

//Q:1.glDeleteVertexArray 一定要调用？？？


void renderScene(Renderer& render, std::string shaderID);
void renderCube(Renderer& render);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool shadows = true;
bool shadowsKeyPressed = false;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Question:
//1.几何着色器  顶点属性块（接口块 不影响GL指令调用）
//2.CubeMap接入？

//一句话概括GS的作用：改变顶点属性的值与顶点组建设的图元类型（一定程度上替代实例化 一个三角形可以弄成多个三角形）

int main()
{
    Renderer render;
    render.initWindows();
    render.initGLFunction();

    render.GetCameraInstance()->Position = glm::vec3(0.0f, 0.0f, 3.0f);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shader("3.2.2.point_shadows.vs", "3.2.2.point_shadows.fs");
    Shader simpleDepthShader("3.2.2.point_shadows_depth.vs", "3.2.2.point_shadows_depth.fs", "3.2.2.point_shadows_depth.gs");

    render.createShader("pointShadow", "3.2.2.point_shadows.vs", "3.2.2.point_shadows.fs");
    render.createShader("pointShadowDepth", "3.2.2.point_shadows_depth.vs", "3.2.2.point_shadows_depth.fs", "3.2.2.point_shadows_depth.gs");

    // load textures
    // -------------
    unsigned int woodTexture = render.loadTexture(FileSystem::getPath("resources/textures/wood.png").c_str());

    // configure depth map FBO
    // -----------------------
    constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO, depthCubemap;
    render.createShadowCubeFrameBuffer(depthMapFBO, depthCubemap, SHADOW_WIDTH, SHADOW_HEIGHT);


    // shader configuration
    // --------------------
    render.getShader("pointShadow")->use();
    render.getShader("pointShadow")->setInt("diffuseTexture", 0);
    render.getShader("pointShadow")->setInt("depthMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    // render loop
    // -----------
    while (!render.isWindowsClosed())
    {
        render.onTimeUpdate();
        render.processWindowsInput();

        // move light position over time
        lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

        // render
        // ------
        render.setClearFlag((uint8_t)ClearFlag::eCOLOR | (uint8_t)ClearFlag::eDEPTH, 0x191919ff);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        render.bindFBufferViewRect(depthMapFBO, 0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        render.setClearFlag((uint8_t)ClearFlag::eDEPTH);
        render.getShader("pointShadowDepth")->use();
        for (unsigned int i = 0; i < 6; ++i)
            render.getShader("pointShadowDepth")->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        render.getShader("pointShadowDepth")->setFloat("far_plane", far_plane);
        render.getShader("pointShadowDepth")->setVec3("lightPos", lightPos);
        renderScene(render, "pointShadowDepth");
       

        // 2. render scene as normal 
        // -------------------------
        uint32_t screenFbo = 0;
        render.bindFBufferViewRect(screenFbo, 0, 0, SCR_WIDTH, SCR_WIDTH);
        render.setClearFlag((uint8_t)ClearFlag::eCOLOR | (uint8_t)ClearFlag::eDEPTH, 0x191919ff);
        render.getShader("pointShadow")->use();
        glm::mat4 projection = glm::perspective(glm::radians(render.GetCameraInstance()->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = render.GetCameraInstance()->GetViewMatrix();
        render.getShader("pointShadow")->setMat4("projection", projection);
        render.getShader("pointShadow")->setMat4("view", view);
        // set lighting uniforms
        render.getShader("pointShadow")->setVec3("lightPos", lightPos);
        render.getShader("pointShadow")->setVec3("viewPos", render.GetCameraInstance()->Position);
        render.getShader("pointShadow")->setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        render.getShader("pointShadow")->setFloat("far_plane", far_plane);
        render.activeAndBindTexture(GL_TEXTURE0, GL_TEXTURE_2D, woodTexture);
        render.activeAndBindTexture(GL_TEXTURE1, GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(render, "pointShadow");

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        render.swapWindowsBuffer();
        render.pollWindowsEvent();
    }

    render.closeWindow();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(Renderer& render, std::string shaderID)
{
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    render.getShader(shaderID)->setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    render.getShader(shaderID)->setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    renderCube(render);
    render.getShader(shaderID)->setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.75f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    model = glm::scale(model, glm::vec3(0.5f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.75f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube(Renderer& render)
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        vector<float> vertices = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        render.createVertexBuffer(cubeVAO, cubeVBO, vertices, {3, 3, 2});
    }
    // render Cube
    render.bindVertexArrayObject(cubeVAO);
    render.drawTriangle(GL_TRIANGLES, 36);
    render.unBindVertexArrayObject();
}


#endif