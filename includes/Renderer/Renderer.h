#pragma once
#include<memory>
#include<map>
#include<vector>
#include<string>

const unsigned int INIT_WIDTH = 800;
const unsigned int INIT_HEIGHT = 600;

enum class ClearFlag
{
	eNONE = 0,
	eCOLOR = 1,
	eDEPTH = 2,
	eSTENCIL = 4
};

class Camera;
class Shader;
typedef struct GLFWwindow GLFWwindow;//结构体的前向声明
class Renderer
{
private:
	GLFWwindow* m_glfwWin = nullptr;
	std::map<std::string, std::shared_ptr<Shader>> m_shaderMap;
	float m_deltaTime = 0.0;
	float m_lastTime = 0.0;
public:
	Renderer();
	~Renderer();

	bool initWindows();
	bool isWindowsClosed();
	void processWindowsInput();
	void swapWindowsBuffer();
	void pollWindowsEvent();
	void closeWindow();

	bool initGLFunction();

	void createVertexBuffer(uint32_t& vao, uint32_t& vbo, std::vector<float>& data, std::vector<int> vtxAttrSizeSet);//float data[] 实际上是 float* data 
	void bindVertexArrayObject(uint32_t vao);
	void unBindVertexArrayObject();

	//void createFrameBuffer(uint32_t& fbHandle, uint32_t& texHandle, int width, int height);

	void createShadowFrameBuffer(uint32_t& fbHandle, uint32_t& texHandle, int width, int height);
	void createShadowCubeFrameBuffer(uint32_t& fbHandle, uint32_t& texHandle, int width, int height);

	//目前是直接bind了framebuffer 和bgfx有区别 我们没有submit才渲染
	void bindFBufferViewRect(uint32_t& fbHandle, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

	void unbindFBuffer();

	void activeTexture(uint32_t texEnum);
	void bindTexture(uint32_t texType, uint32_t texId);

	void activeAndBindTexture(uint32_t texEnum, uint32_t texType, uint32_t texId);

	void drawTriangle(uint32_t triType, int vtxCount);

	void onTimeUpdate();

	float getCurrentTime();

	//void createIndexBuffer();
	//void createUniform();
	//void setVertexBuffer();
	//void setIndexBuffer();
	//void setUniform();
	//void setTexture();

	//调用了glclear指令的
	/*
	* flag 00000000  最后三位表示开启stencil depth color
	*/
	void setClearFlag(uint8_t flag, uint32_t rgba = 0x000000ff, float depth = 1.0f, uint32_t stencil = 0);

	void createShader(std::string str, std::string vertex, std::string frag, std::string geometry = "");
	std::shared_ptr<Shader> getShader(std::string str);

	uint32_t loadTexture(char const* path, bool gammaCorrection = false);
	uint32_t loadCubemap(std::vector<std::string> faces);

public:

	static Camera* GetCameraInstance();
};

