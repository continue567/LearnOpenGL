
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

//Question:1.正交投影和透视投影的区别是什么 投影到使椎体裁剪再到视口变换 具体的计算过程？
//为什么如果生成深度图用透视投影的时候 生成深度图需要把z值转为线性的（调试的时候 实际使用还是要用非线性的）
//2.阴影贴图效果若干优化的点？（考虑光照和片元法线夹角的阴影偏移、PCF邻近采样深度图取平均）
//3.阴影粉刺原理细节？


void renderScene(Renderer& render, std::string shaderID);
void renderCube(Renderer& render);
void renderQuad(Renderer& render);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


// meshes
unsigned int planeVAO, planeVBO;

int main()
{
    // glfw: initialize and configure
    Renderer render;
    render.initWindows();
    render.initGLFunction();

    render.GetCameraInstance()->Position = glm::vec3(0.0f, 0.0f, 3.0f);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    render.createShader("shadowMap", "3.1.3.shadow_mapping.vs", "3.1.3.shadow_mapping.fs");
    render.createShader("shadowMapDepth", "3.1.3.shadow_mapping_depth.vs", "3.1.3.shadow_mapping_depth.fs");
    render.createShader("debugQuad", "3.1.3.debug_quad.vs", "3.1.3.debug_quad_depth.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    vector<float> planeVertices = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    // plane VAO
    //unsigned int planeVBO;
    render.createVertexBuffer(planeVAO, planeVBO, planeVertices, { 3,3,2 });
    // load textures
    // -------------
    //unsigned int woodTexture = loadTexture(FileSystem::getPath("resources/textures/wood.png").c_str());
    unsigned int woodTexture = render.loadTexture(FileSystem::getPath("resources/textures/wood.png").c_str());

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO, depthMap;
    render.createShadowFrameBuffer(depthMapFBO, depthMap, 1024, 1024);
    //GLenum errCde = glGetError();

    // shader configuration
    // --------------------
    render.getShader("shadowMap")->use();
    render.getShader("shadowMap")->setInt("diffuseTexture", 0);
    render.getShader("shadowMap")->setInt("shadowMap", 1);

    render.getShader("debugQuad")->use();
    render.getShader("debugQuad")->setInt("depthMap", 0);

    // lighting info
    // -------------
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

    // render loop
    // -----------
    while (!render.isWindowsClosed())
    {
        // per-frame time logic
        // --------------------
        /*float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;*/
        render.onTimeUpdate();

        // input
        // -----
        //processInput(window);
        render.processWindowsInput();

        // change light position over time
        //lightPos.x = sin(glfwGetTime()) * 3.0f;
        //lightPos.z = cos(glfwGetTime()) * 2.0f;
        //lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

        // render
        // ------
        //0x191919ff
        render.setClearFlag((uint8_t)ClearFlag::eCOLOR | (uint8_t)ClearFlag::eDEPTH, 0x191919ff);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        render.getShader("shadowMapDepth")->use();
        render.getShader("shadowMapDepth")->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        render.bindFBufferViewRect(depthMapFBO, 0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        render.setClearFlag((uint8_t)ClearFlag::eDEPTH);
        //render.activeAndBindTexture(GL_TEXTURE0, GL_TEXTURE_2D, woodTexture);
        renderScene(render, "shadowMapDepth");
       

        // reset viewport
        uint32_t screenFbHandle = 0;
        render.bindFBufferViewRect(screenFbHandle, 0, 0, SCR_WIDTH, SCR_HEIGHT);
        render.setClearFlag((uint8_t)ClearFlag::eDEPTH | (uint8_t)ClearFlag::eCOLOR, 0x191919ff);

        // 2. render scene as normal using the generated depth/shadow map  
        // --------------------------------------------------------------
        render.getShader("shadowMap")->use();
        glm::mat4 projection = glm::perspective(glm::radians(render.GetCameraInstance()->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = render.GetCameraInstance()->GetViewMatrix();
        render.getShader("shadowMap")->setMat4("projection", projection);
        render.getShader("shadowMap")->setMat4("view", view);
        // set light uniforms
        render.getShader("shadowMap")->setVec3("viewPos", render.GetCameraInstance()->Position);
        render.getShader("shadowMap")->setVec3("lightPos", lightPos);
        render.getShader("shadowMap")->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        render.activeAndBindTexture(GL_TEXTURE0, GL_TEXTURE_2D, woodTexture);
        render.activeAndBindTexture(GL_TEXTURE1, GL_TEXTURE_2D, depthMap);
        renderScene(render, "shadowMap");

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        //render.getShader("debugQuad")->use();
        //render.getShader("debugQuad")->setFloat("near_plane", near_plane);
        //render.getShader("debugQuad")->setFloat("far_plane", far_plane);
        //render.activeAndBindTexture(GL_TEXTURE0, GL_TEXTURE_2D, depthMap);
        //renderQuad(render);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        render.swapWindowsBuffer();
        render.pollWindowsEvent();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    render.closeWindow();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(Renderer& render, std::string shaderID)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    render.getShader(shaderID)->setMat4("model", model);
    render.bindVertexArrayObject(planeVAO);
    render.drawTriangle(GL_TRIANGLES, 6);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    render.getShader(shaderID)->setMat4("model", model);
    renderCube(render);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
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
        std::vector<float> vertices = {
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
        render.createVertexBuffer(cubeVAO, cubeVBO, vertices, { 3, 3, 2 });
    }
    render.bindVertexArrayObject(cubeVAO);
    render.drawTriangle(GL_TRIANGLES, 36);
    render.unBindVertexArrayObject();
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(Renderer& render)
{
    if (quadVAO == 0)
    {
        std::vector<float> quadVertices = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        render.createVertexBuffer(quadVAO, quadVBO, quadVertices, { 3, 2 });
    }
    render.bindVertexArrayObject(quadVAO);
    render.drawTriangle(GL_TRIANGLE_STRIP, 4);
    render.unBindVertexArrayObject();
}

#endif