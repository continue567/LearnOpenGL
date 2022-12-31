#pragma once
#include<memory>
#include<map>
#include<vector>
#include<string>

const unsigned int INIT_WIDTH = 720;
const unsigned int INIT_HEIGHT = 1280;

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

	void activeTexture(uint32_t texEnum);
	void bindTexture(uint32_t texType, uint32_t texId);

	void drawTriangle(int vtxCount);

	void onTimeUpdate();

	//void createIndexBuffer();
	//void createUniform();
	//void setVertexBuffer();
	//void setIndexBuffer();
	//void setUniform();
	//void setTexture();

	void setClearFlag(uint8_t flag, uint32_t rgba = 0x000000ff, float depth = 1.0f, uint32_t stencil = 0);

	void createShader(std::string str, std::string vertex, std::string frag);
	std::shared_ptr<Shader> getShader(std::string str);

	uint32_t loadTexture(char const* path);
	uint32_t loadCubemap(std::vector<std::string> faces);

public:

	static Camera* GetCameraInstance();
};

