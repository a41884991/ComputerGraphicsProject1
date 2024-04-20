#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <cmath>
#include <filesystem>

#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define FLOOR

#define MODEL_PARTS_NUM 17
#define X_AXIS glm::vec3(1.0f,0.0f,0.0f)
#define Y_AXIS glm::vec3(0.0f,1.0f,0.0f)
#define Z_AXIS glm::vec3(0.0f,0.0f,1.0f)

#define BODY_UP			0
#define BODY_DOWN		1
#define HEAD			2
#define LEFT_SHOULDER	3
#define LEFT_ARM_UP		4
#define LEFT_ARM_DOWN	5
#define LEFT_HAND		6
#define LEFT_LEG_UP		7
#define LEFT_LEG_DOWN	8
#define LEFT_FOOT		9
#define RIGHT_SHOULDER	10
#define RIGHT_ARM_UP	11
#define RIGHT_ARM_DOWN	12
#define RIGHT_HAND		13
#define RIGHT_LEG_UP	14
#define RIGHT_LEG_DOWN	15
#define RIGHT_FOOT		16

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void OnMouseCursorPos(GLFWwindow* window, double xPos, double yPos);
void OnMouse(GLFWwindow* window, int button, int action, int mods);
std::vector<Model> loadModel();
void setLightSetting(Shader& shader, bool* lightOn, float* dirLightColor, float* pointLightColor, float* spotLightColor);
glm::vec3 arrayToVec3(const float* array);

void updateModel();

namespace Action {
	void ChooseAction(int& index);

	void ResetRotation();

	void Idle();

	void Walk(int& step);
	void JumpingJack(int& step);
	void Squad(int& step);
	void KamaHameHa(int& step);
	void BuuVictory(int& step);
	void Moonwalk(int& step);
	void GangnamSytle(int& step);
	void SitUp(int& step);
	void PushUp(int& step);

}

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

bool enableMouseMovement = false;

Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int tempActionSpeed = 20;
int actionSpeed = 20;

int testX, testY;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

glm::vec3 fullBodyMovement;
glm::vec3 fullBodyRotation;

const std::vector<glm::vec3> modelPos = {
	glm::vec3(0.0f, 0.84f, 0.0f),
	glm::vec3(0.0f, -0.085f, -0.03f),
	glm::vec3(0.0f, 0.05f, 0.06f),
	glm::vec3(0.13f, 0.06f, 0.0f),
	glm::vec3(0.13f, 0.06f, 0.0f),
	glm::vec3(0.0f, -0.115f, 0.0f),
	glm::vec3(0.0f, -0.155f, 0.0f),
	glm::vec3(0.1f, -0.105f, 0.0f),
	glm::vec3(0.0f, -0.27f, 0.0f),
	glm::vec3(0.0f, -0.325f, 0.0f),
	glm::vec3(-0.13f, 0.06f, 0.0f),
	glm::vec3(-0.13f, 0.06f, 0.0f),
	glm::vec3(0.0f, -0.125f, 0.0f),
	glm::vec3(0.03f, -0.135f, 0.01f),
	glm::vec3(-0.1f, -0.11f, 0.0f),
	glm::vec3(0.0f, -0.27f, 0.0f),
	glm::vec3(0.0f, -0.3f, 0.0f)
};

std::vector<glm::mat4> modelMat(MODEL_PARTS_NUM);

std::vector<glm::vec3> modelAngles(MODEL_PARTS_NUM);

int main()
{
	//初始化並指定openGL版本
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project1", NULL, NULL);
	if (!window)
	{
		std::cout << "Fail to create window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	//註冊FramebufferSizeCallback function
	glfwSetCursorPosCallback(window, OnMouseCursorPos);
	glfwSetMouseButtonCallback(window, OnMouse);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Fail to initilaize glad\n";
		return -1;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_DEPTH_TEST);	//啟用深度測試
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Shader shader("shader/modelVS.vs", "shader/modelFS.fs");

	std::vector<Model> models = loadModel();

	Model stage("stage/stage.obj");

	glm::mat4 stageModel = glm::mat4(1.0f);
	stageModel = glm::translate(stageModel, glm::vec3(0.0f, -1.5f, 0.0f));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	float dirLightColor[] = { 1.0f,1.0f,1.0f };
	float pointLightColor[] = { 1.0f,1.0f,1.0f };
	float spotLightColor[] = { 1.0f,1.0f,1.0f };

	bool lightOn[] = { true,true,true }; // 0=dirLight 1=pointLight 2=spotLight

	double lastTime = glfwGetTime();
	int nbFrames = 0, displayFrames = 0;

	const char* actionOptions[] = { "Idle","Walk","Jumping Jack","Squad","Kame Hame Ha","Buu Victory","Moonwalk","Gangnam Style","Sit-up","Push-up" };
	int actionNum = 0;

	while (!glfwWindowShouldClose(window))
	{
		//時間標準化
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		nbFrames++;
		if (currentFrame - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
			displayFrames = nbFrames;
			nbFrames = 0;
			lastTime += 1.0;
		}


		//處理input
		processInput(window);

		//渲染背景
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();

			ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
			ImGui::Begin("Controll");

			ImGui::Checkbox("Directional Light", &lightOn[0]);
			ImGui::ColorEdit3("Directional Light Color", dirLightColor);
			ImGui::Checkbox("Point Light", &lightOn[1]);
			ImGui::ColorEdit3("Point Light Color", pointLightColor);
			ImGui::Checkbox("Spot Light", &lightOn[2]);
			ImGui::ColorEdit3("Spot Light Color", spotLightColor);

			if (ImGui::BeginCombo("Action", actionOptions[actionNum])) {
				for (int i = 0; i < IM_ARRAYSIZE(actionOptions); i++) {
					bool isSelected = (actionNum == i);
					if (ImGui::Selectable(actionOptions[i], isSelected))
						actionNum = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::SliderInt("Speed", &actionSpeed, 40, 10, "");

			{
				if (ImGui::Button("Front"))
				{
					camera.Position = glm::vec3(0.0f, 0.5f, 3.0f);
					camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
					camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
					camera.Yaw = 270.0f;
					camera.Pitch = 0.0f;
					firstMouse = true;
				}
				ImGui::SameLine();

				if (ImGui::Button("Back"))
				{
					camera.Position = glm::vec3(0.0f, 0.5f, -3.0f);
					camera.Front = glm::vec3(0.0f, 0.0f, 1.0f);
					camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
					camera.Yaw = 90.0f;
					camera.Pitch = 0.0f;
					firstMouse = true;
				}
				ImGui::SameLine();

				if (ImGui::Button("Right"))
				{
					camera.Position = glm::vec3(3.0f, 0.5f, 0.0f);
					camera.Front = glm::vec3(-1.0f, 0.0f, 0.0f);
					camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
					camera.Yaw = 180.0f;
					camera.Pitch = 0.0f;
					firstMouse = true;
				}
				ImGui::SameLine();

				if (ImGui::Button("Left"))
				{
					camera.Position = glm::vec3(-3.0f, 0.5f, 0.0f);
					camera.Front = glm::vec3(1.0f, 0.0f, 0.0f);
					camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
					camera.Yaw = 0.0f;
					camera.Pitch = 0.0f;
					firstMouse = true;
				}
				ImGui::SameLine();
				ImGui::Text("View");
			}

			ImGui::Text("FPS: %d", displayFrames);

			ImGui::End();
		}

		Action::ChooseAction(actionNum);

		shader.use();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.f);
		glm::mat4 view = camera.GetViewMatrix();

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		setLightSetting(shader, lightOn, dirLightColor, pointLightColor, spotLightColor);

		updateModel();

		for (int i = 0; i < MODEL_PARTS_NUM; i++) {

			shader.setMat4("model", modelMat[i]);

			glm::mat4 normalMat = glm::transpose(glm::inverse(modelMat[i]));
			shader.setMat4("normalMat", normalMat);

			models[i].Draw(shader);
		}

		shader.setMat4("model", stageModel);
		stage.Draw(shader);

#ifdef FLOOR
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		floorShader.use();
		floorShader.setMat4("projection", projection);
		floorShader.setMat4("view", view);
		floorShader.setMat4("model", model);

		floorVAO.Bind();
		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
		floorVAO.Unbind();

#endif // FLOOR


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void OnMouseCursorPos(GLFWwindow* window, double xPos, double yPos)
{
	static double lastX;
	static double lastY;

	if (firstMouse || !enableMouseMovement)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
		return;
	}

	float xOffset = xPos - lastX; // reversed since y-coordinates go from bottom to top
	float yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void OnMouse(GLFWwindow* window, int button, int action, int mods)
{
	const ImGuiIO& io = ImGui::GetIO();

	if (!io.WantCaptureMouse)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			enableMouseMovement = true;
		else
			enableMouseMovement = false;
	}

}

std::vector<Model> loadModel()
{
	std::vector<Model> models = {

		Model("model/bodyUp.obj"),
		Model("model/bodyDown.obj"),
		Model("model/head.obj"),
		Model("model/leftShoulder.obj"),
		Model("model/leftArmUp.obj"),
		Model("model/leftArmDown.obj"),
		Model("model/leftHand.obj"),
		Model("model/leftLegUp.obj"),
		Model("model/leftLegDown.obj"),
		Model("model/leftFoot.obj"),
		Model("model/rightShoulder.obj"),
		Model("model/rightArmUp.obj"),
		Model("model/rightArmDown.obj"),
		Model("model/rightHand.obj"),
		Model("model/rightLegUp.obj"),
		Model("model/rightLegDown.obj"),
		Model("model/rightFoot.obj")
	};

	return models;
}

void setLightSetting(Shader& shader, bool* lightOn, float* dirLightColor, float* pointLightColor, float* spotLightColor)
{
	shader.setBool("lightController.dirLightOn", lightOn[0]);
	shader.setBool("lightController.pointLightOn", lightOn[1]);
	shader.setBool("lightController.spotLightOn", lightOn[2]);

	shader.setVec3("dirLight.ambient", dirLightColor[0], dirLightColor[1], dirLightColor[2]);
	shader.setVec3("dirLight.diffuse", dirLightColor[0], dirLightColor[1], dirLightColor[2]);
	shader.setVec3("dirLight.specular", dirLightColor[0], dirLightColor[1], dirLightColor[2]);
	shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);


	shader.setVec3("pointLight.ambient", pointLightColor[0], pointLightColor[1], pointLightColor[2]);
	shader.setVec3("pointLight.diffuse", pointLightColor[0], pointLightColor[1], pointLightColor[2]);
	shader.setVec3("pointLight.specular", pointLightColor[0], pointLightColor[1], pointLightColor[2]);
	shader.setVec3("pointLight.position", lightPos);
	shader.setFloat("pointLight.constant", 1.0f);
	shader.setFloat("pointLight.linear", 0.09f);
	shader.setFloat("pointLight.quadratic", 0.032f);

	shader.setVec3("spotLight.ambient", spotLightColor[0], spotLightColor[1], spotLightColor[2]);
	shader.setVec3("spotLight.diffuse", spotLightColor[0], spotLightColor[1], spotLightColor[2]);
	shader.setVec3("spotLight.specular", spotLightColor[0], spotLightColor[1], spotLightColor[2]);
	shader.setVec3("spotLight.position", 0.0f, 2.0f, 0.0f);
	shader.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
	shader.setFloat("spotLight.constant", 1.0f);
	shader.setFloat("spotLight.linear", 0.09f);
	shader.setFloat("spotLight.quadratic", 0.032f);
	shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
}

glm::vec3 arrayToVec3(const float* array)
{
	return glm::vec3(array[0], array[1], array[2]);
}

void updateModel() {
	glm::mat4 rotation[MODEL_PARTS_NUM];
	glm::mat4 translation[MODEL_PARTS_NUM];

	for (int i = 0; i < MODEL_PARTS_NUM; i++) {
		modelMat[i] = glm::mat4(1.0f);
		rotation[i] = glm::mat4(1.0f);
		translation[i] = glm::mat4(1.0f);
	}

	float pitch, yaw, roll;

	//body Up 基準(身體上半)
	//根據yaw-pitch-roll進行旋轉(Y_X_Z)
	rotation[BODY_UP] = glm::rotate(rotation[BODY_UP], glm::radians(fullBodyRotation.z), Z_AXIS);
	rotation[BODY_UP] = glm::rotate(rotation[BODY_UP], glm::radians(fullBodyRotation.x), X_AXIS);
	rotation[BODY_UP] = glm::rotate(rotation[BODY_UP], glm::radians(fullBodyRotation.y), Y_AXIS);
	translation[BODY_UP] = glm::translate(translation[BODY_UP], fullBodyMovement + modelPos[BODY_UP]);
	modelMat[BODY_UP] = translation[BODY_UP] * rotation[BODY_UP];
	//==================
	//body Down
	rotation[BODY_DOWN] = glm::rotate(rotation[BODY_DOWN], glm::radians(modelAngles[BODY_DOWN].z), Z_AXIS);
	rotation[BODY_DOWN] = glm::rotate(rotation[BODY_DOWN], glm::radians(modelAngles[BODY_DOWN].x), X_AXIS);
	rotation[BODY_DOWN] = glm::rotate(rotation[BODY_DOWN], glm::radians(modelAngles[BODY_DOWN].y), Y_AXIS);
	translation[BODY_DOWN] = glm::translate(translation[BODY_DOWN], modelPos[BODY_DOWN]);
	modelMat[BODY_DOWN] = modelMat[BODY_UP] * translation[BODY_DOWN] * rotation[BODY_DOWN];
	//==================
	//head
	rotation[HEAD] = glm::rotate(rotation[HEAD], glm::radians(modelAngles[HEAD].z), Z_AXIS);
	rotation[HEAD] = glm::rotate(rotation[HEAD], glm::radians(modelAngles[HEAD].x), X_AXIS);
	rotation[HEAD] = glm::rotate(rotation[HEAD], glm::radians(modelAngles[HEAD].y), Y_AXIS);
	translation[HEAD] = glm::translate(translation[HEAD], modelPos[HEAD]);
	modelMat[HEAD] = modelMat[BODY_UP] * translation[HEAD] * rotation[HEAD];
	//================
	//left shoulder
	yaw = glm::radians(modelAngles[LEFT_SHOULDER].y);
	pitch = glm::radians(modelAngles[LEFT_SHOULDER].x < -55.0f ? -55.0f : modelAngles[LEFT_SHOULDER].x);
	roll = glm::radians(modelAngles[LEFT_SHOULDER].z > 55.0f ? 55.0f : modelAngles[LEFT_SHOULDER].z);
	rotation[LEFT_SHOULDER] = glm::rotate(rotation[LEFT_SHOULDER], roll, Z_AXIS);
	rotation[LEFT_SHOULDER] = glm::rotate(rotation[LEFT_SHOULDER], pitch, X_AXIS);
	//rotation[LEFT_SHOULDER] = glm::rotate(rotation[LEFT_SHOULDER], yaw, Y_AXIS);
	translation[LEFT_SHOULDER] = glm::translate(translation[LEFT_SHOULDER], modelPos[LEFT_SHOULDER]);
	modelMat[LEFT_SHOULDER] = modelMat[BODY_UP] * translation[LEFT_SHOULDER] * rotation[LEFT_SHOULDER];
	//================
	//left arm up
	rotation[LEFT_ARM_UP] = glm::rotate(rotation[LEFT_ARM_UP], glm::radians(modelAngles[LEFT_ARM_UP].z), Z_AXIS);
	rotation[LEFT_ARM_UP] = glm::rotate(rotation[LEFT_ARM_UP], glm::radians(modelAngles[LEFT_ARM_UP].x), X_AXIS);
	rotation[LEFT_ARM_UP] = glm::rotate(rotation[LEFT_ARM_UP], glm::radians(modelAngles[LEFT_ARM_UP].y), Y_AXIS);
	translation[LEFT_ARM_UP] = glm::translate(translation[LEFT_ARM_UP], modelPos[LEFT_ARM_UP]);
	modelMat[LEFT_ARM_UP] = modelMat[BODY_UP] * translation[LEFT_ARM_UP] * rotation[LEFT_ARM_UP];
	//================
	//left arm down
	rotation[LEFT_ARM_DOWN] = glm::rotate(rotation[LEFT_ARM_DOWN], glm::radians(modelAngles[LEFT_ARM_DOWN].z), Z_AXIS);
	rotation[LEFT_ARM_DOWN] = glm::rotate(rotation[LEFT_ARM_DOWN], glm::radians(modelAngles[LEFT_ARM_DOWN].x), X_AXIS);
	rotation[LEFT_ARM_DOWN] = glm::rotate(rotation[LEFT_ARM_DOWN], glm::radians(modelAngles[LEFT_ARM_DOWN].y), Y_AXIS);
	translation[LEFT_ARM_DOWN] = glm::translate(translation[LEFT_ARM_DOWN], modelPos[LEFT_ARM_DOWN]);
	modelMat[LEFT_ARM_DOWN] = modelMat[LEFT_ARM_UP] * translation[LEFT_ARM_DOWN] * rotation[LEFT_ARM_DOWN];
	//=================
	// left hand
	rotation[LEFT_HAND] = glm::rotate(rotation[LEFT_HAND], glm::radians(modelAngles[LEFT_HAND]).z, Z_AXIS);
	translation[LEFT_HAND] = glm::translate(translation[LEFT_HAND], modelPos[LEFT_HAND]);
	modelMat[LEFT_HAND] = modelMat[LEFT_ARM_DOWN] * translation[LEFT_HAND] * rotation[LEFT_HAND];
	//=================
	//left leg up
	rotation[LEFT_LEG_UP] = glm::rotate(rotation[LEFT_LEG_UP], glm::radians(modelAngles[LEFT_LEG_UP].z), Z_AXIS);
	rotation[LEFT_LEG_UP] = glm::rotate(rotation[LEFT_LEG_UP], glm::radians(modelAngles[LEFT_LEG_UP].x), X_AXIS);
	rotation[LEFT_LEG_UP] = glm::rotate(rotation[LEFT_LEG_UP], glm::radians(modelAngles[LEFT_LEG_UP].y), Y_AXIS);
	translation[LEFT_LEG_UP] = glm::translate(translation[LEFT_LEG_UP], modelPos[LEFT_LEG_UP]);
	modelMat[LEFT_LEG_UP] = modelMat[BODY_DOWN] * translation[LEFT_LEG_UP] * rotation[LEFT_LEG_UP];
	//=================
	//left leg down
	rotation[LEFT_LEG_DOWN] = glm::rotate(rotation[LEFT_LEG_DOWN], glm::radians(modelAngles[LEFT_LEG_DOWN].z), Z_AXIS);
	rotation[LEFT_LEG_DOWN] = glm::rotate(rotation[LEFT_LEG_DOWN], glm::radians(modelAngles[LEFT_LEG_DOWN].x), X_AXIS);
	rotation[LEFT_LEG_DOWN] = glm::rotate(rotation[LEFT_LEG_DOWN], glm::radians(modelAngles[LEFT_LEG_DOWN].y), Y_AXIS);
	translation[LEFT_LEG_DOWN] = glm::translate(translation[LEFT_LEG_DOWN], modelPos[LEFT_LEG_DOWN]);
	modelMat[LEFT_LEG_DOWN] = modelMat[LEFT_LEG_UP] * translation[LEFT_LEG_DOWN] * rotation[LEFT_LEG_DOWN];
	//=================
	// left foot
	rotation[LEFT_FOOT] = glm::rotate(rotation[LEFT_FOOT], glm::radians(modelAngles[LEFT_FOOT].z), Z_AXIS);
	rotation[LEFT_FOOT] = glm::rotate(rotation[LEFT_FOOT], glm::radians(modelAngles[LEFT_FOOT].x), X_AXIS);
	rotation[LEFT_FOOT] = glm::rotate(rotation[LEFT_FOOT], glm::radians(modelAngles[LEFT_FOOT].y), Y_AXIS);
	translation[LEFT_FOOT] = glm::translate(translation[LEFT_FOOT], modelPos[LEFT_FOOT]);
	modelMat[LEFT_FOOT] = modelMat[LEFT_LEG_DOWN] * translation[LEFT_FOOT] * rotation[LEFT_FOOT];
	//=================
	//right shoulder
	yaw = glm::radians(modelAngles[RIGHT_SHOULDER].y);
	pitch = glm::radians(modelAngles[RIGHT_SHOULDER].x < -55.0f ? -55.0f : modelAngles[RIGHT_SHOULDER].x);
	roll = glm::radians(modelAngles[RIGHT_SHOULDER].z < -55.0f ? -55.0f : modelAngles[RIGHT_SHOULDER].z);
	rotation[RIGHT_SHOULDER] = glm::rotate(rotation[RIGHT_SHOULDER], roll, Z_AXIS);
	rotation[RIGHT_SHOULDER] = glm::rotate(rotation[RIGHT_SHOULDER], pitch, X_AXIS);
	//rotation[RIGHT_SHOULDER] = glm::rotate(rotation[RIGHT_SHOULDER], yaw, Y_AXIS);
	translation[RIGHT_SHOULDER] = glm::translate(translation[RIGHT_SHOULDER], modelPos[RIGHT_SHOULDER]);
	modelMat[RIGHT_SHOULDER] = modelMat[BODY_UP] * translation[RIGHT_SHOULDER] * rotation[RIGHT_SHOULDER];
	//===================
	//right arm up
	rotation[RIGHT_ARM_UP] = glm::rotate(rotation[RIGHT_ARM_UP], glm::radians(modelAngles[RIGHT_ARM_UP].z), Z_AXIS);
	rotation[RIGHT_ARM_UP] = glm::rotate(rotation[RIGHT_ARM_UP], glm::radians(modelAngles[RIGHT_ARM_UP].x), X_AXIS);
	rotation[RIGHT_ARM_UP] = glm::rotate(rotation[RIGHT_ARM_UP], glm::radians(modelAngles[RIGHT_ARM_UP].y), Y_AXIS);
	translation[RIGHT_ARM_UP] = glm::translate(translation[RIGHT_ARM_UP], modelPos[RIGHT_ARM_UP]);
	modelMat[RIGHT_ARM_UP] = modelMat[BODY_UP] * translation[RIGHT_ARM_UP] * rotation[RIGHT_ARM_UP];
	//================
	//right arm down
	rotation[RIGHT_ARM_DOWN] = glm::rotate(rotation[RIGHT_ARM_DOWN], glm::radians(modelAngles[RIGHT_ARM_DOWN].z), Z_AXIS);
	rotation[RIGHT_ARM_DOWN] = glm::rotate(rotation[RIGHT_ARM_DOWN], glm::radians(modelAngles[RIGHT_ARM_DOWN].x), X_AXIS);
	rotation[RIGHT_ARM_DOWN] = glm::rotate(rotation[RIGHT_ARM_DOWN], glm::radians(modelAngles[RIGHT_ARM_DOWN].y), Y_AXIS);
	translation[RIGHT_ARM_DOWN] = glm::translate(translation[RIGHT_ARM_DOWN], modelPos[RIGHT_ARM_DOWN]);
	modelMat[RIGHT_ARM_DOWN] = modelMat[RIGHT_ARM_UP] * translation[RIGHT_ARM_DOWN] * rotation[RIGHT_ARM_DOWN];
	//=================
	// right hand
	rotation[RIGHT_HAND] = glm::rotate(rotation[RIGHT_HAND], glm::radians(modelAngles[RIGHT_HAND]).z, Z_AXIS);
	translation[RIGHT_HAND] = glm::translate(translation[RIGHT_HAND], modelPos[RIGHT_HAND]);
	modelMat[RIGHT_HAND] = modelMat[RIGHT_ARM_DOWN] * translation[RIGHT_HAND] * rotation[RIGHT_HAND];
	//=================
	//right leg up
	rotation[RIGHT_LEG_UP] = glm::rotate(rotation[RIGHT_LEG_UP], glm::radians(modelAngles[RIGHT_LEG_UP].z), Z_AXIS);
	rotation[RIGHT_LEG_UP] = glm::rotate(rotation[RIGHT_LEG_UP], glm::radians(modelAngles[RIGHT_LEG_UP].x), X_AXIS);
	rotation[RIGHT_LEG_UP] = glm::rotate(rotation[RIGHT_LEG_UP], glm::radians(modelAngles[RIGHT_LEG_UP].y), Y_AXIS);
	translation[RIGHT_LEG_UP] = glm::translate(translation[RIGHT_LEG_UP], modelPos[RIGHT_LEG_UP]);
	modelMat[RIGHT_LEG_UP] = modelMat[BODY_DOWN] * translation[RIGHT_LEG_UP] * rotation[RIGHT_LEG_UP];
	//=================
	//right leg down
	rotation[RIGHT_LEG_DOWN] = glm::rotate(rotation[RIGHT_LEG_DOWN], glm::radians(modelAngles[RIGHT_LEG_DOWN].z), Z_AXIS);
	rotation[RIGHT_LEG_DOWN] = glm::rotate(rotation[RIGHT_LEG_DOWN], glm::radians(modelAngles[RIGHT_LEG_DOWN].x), X_AXIS);
	rotation[RIGHT_LEG_DOWN] = glm::rotate(rotation[RIGHT_LEG_DOWN], glm::radians(modelAngles[RIGHT_LEG_DOWN].y), Y_AXIS);
	translation[RIGHT_LEG_DOWN] = glm::translate(translation[RIGHT_LEG_DOWN], modelPos[RIGHT_LEG_DOWN]);
	modelMat[RIGHT_LEG_DOWN] = modelMat[RIGHT_LEG_UP] * translation[RIGHT_LEG_DOWN] * rotation[RIGHT_LEG_DOWN];
	//=================
	//right foot
	rotation[RIGHT_FOOT] = glm::rotate(rotation[RIGHT_FOOT], glm::radians(modelAngles[RIGHT_FOOT].z), Z_AXIS);
	rotation[RIGHT_FOOT] = glm::rotate(rotation[RIGHT_FOOT], glm::radians(modelAngles[RIGHT_FOOT].x), X_AXIS);
	rotation[RIGHT_FOOT] = glm::rotate(rotation[RIGHT_FOOT], glm::radians(modelAngles[RIGHT_FOOT].y), Y_AXIS);
	translation[RIGHT_FOOT] = glm::translate(translation[RIGHT_FOOT], modelPos[RIGHT_FOOT]);
	modelMat[RIGHT_FOOT] = modelMat[RIGHT_LEG_DOWN] * translation[RIGHT_FOOT] * rotation[RIGHT_FOOT];
	//=================
}

void Action::ChooseAction(int& index)
{
	static int priviousIndex = 0;
	static int step;
	if (priviousIndex != index) {
		step = 0;
		priviousIndex = index;
		fullBodyRotation = glm::vec3(0.0f, 0.0f, 0.0f);
		fullBodyMovement = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	if (step == 0) {
		tempActionSpeed = actionSpeed;
	}

	switch (index)
	{
	case 0:
		Idle();
		break;
	case 1:
		Walk(step);
		break;
	case 2:
		JumpingJack(step);
		break;
	case 3:
		Squad(step);
		break;
	case 4:
		KamaHameHa(step);
		break;
	case 5:
		BuuVictory(step);
		break;
	case 6:
		Moonwalk(step);
		break;
	case 7:
		GangnamSytle(step);
		break;
	case 8:
		SitUp(step);
		break;
	case 9:
		PushUp(step);
		break;
	default:
		Idle();
	}
}

void Action::ResetRotation()
{
	for (int i = 0; i < MODEL_PARTS_NUM; i++) {
		modelAngles[i] = glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

void Action::Idle() {
	for (int i = 0; i < MODEL_PARTS_NUM; i++) {
		modelAngles[i] = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	fullBodyRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	fullBodyMovement = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Action::Walk(int& step)
{
	switch (step) {
	case 0:
		step = 1;
		ResetRotation();
		break;

	case 1:
		modelAngles[RIGHT_ARM_UP].x += -50.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].x += 25.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].x += -50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 50.0f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -50.0f)
			step = 2;
		break;

	case 2:
		modelAngles[RIGHT_ARM_UP].x += 50.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].x += -25.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].x += 50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -50.0f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= 0.0f)
			step = 3;
		break;

	case 3:
		modelAngles[LEFT_ARM_UP].x += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].x += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 50.0f / tempActionSpeed;

		if (modelAngles[LEFT_ARM_UP].x <= -50.0f)
			step = 4;
		break;

	case 4:
		modelAngles[LEFT_ARM_UP].x += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += -25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].x += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -50.0f / tempActionSpeed;

		if (modelAngles[LEFT_ARM_UP].x >= 0.0f)
			step = 0;
		break;
	}

	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];

	fullBodyMovement.z += 0.1 / tempActionSpeed;

	if (fullBodyMovement.z >= 2)
		fullBodyMovement.z = 0;
}

void Action::JumpingJack(int& step)
{
	static float movementSpeedY;

	switch (step)
	{
	case 0:
		Idle();
		step = 1;
		movementSpeedY = 0.2 / tempActionSpeed;
		break;

	case 1:
		modelAngles[LEFT_ARM_UP].y += 180.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -180.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -90.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -22.5f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 22.5f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -10.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += 10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -10.0f / tempActionSpeed;

		fullBodyMovement.y += movementSpeedY;
		movementSpeedY += -0.1 / tempActionSpeed / (2 * tempActionSpeed - 1);

		if (modelAngles[LEFT_ARM_UP].y >= 180.0f)
			step = 2;
		break;

	case 2:
		modelAngles[LEFT_ARM_UP].z += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -90.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -22.5f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 22.5f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -10.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += 10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -10.0f / tempActionSpeed;

		fullBodyMovement.y += movementSpeedY;
		movementSpeedY += -0.1 / tempActionSpeed / (2 * tempActionSpeed - 1);

		if (modelAngles[LEFT_ARM_UP].z >= 180.0f)
			step = 3;
		break;

	case 3:
		fullBodyMovement.y -= 0.75 / tempActionSpeed;

		if (fullBodyMovement.y <= -0.02f) {
			movementSpeedY = 0.22f / tempActionSpeed;
			step = 4;
		}

		break;

	case 4:
		modelAngles[LEFT_ARM_UP].y += -180.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 180.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 45.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -45.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += -90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 10.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += -10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 10.0f / tempActionSpeed;

		fullBodyMovement.y += movementSpeedY;
		movementSpeedY += -0.11f / tempActionSpeed / (2 * tempActionSpeed - 1);

		if (modelAngles[RIGHT_ARM_UP].y >= 0.0f)
			step = 5;
		break;

	case 5:
		modelAngles[LEFT_ARM_UP].z += -90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 10.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += -10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 10.0f / tempActionSpeed;

		fullBodyMovement.y += movementSpeedY;
		movementSpeedY += -0.11f / tempActionSpeed / (2 * tempActionSpeed - 1);

		if (modelAngles[RIGHT_ARM_UP].z >= 0.0f)
			step = 6;
		break;

	case 6:
		fullBodyMovement.y -= 0.75 / tempActionSpeed;

		if (fullBodyMovement.y <= 0.0f)
			step = 0;
		break;
	}

	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
}

void Action::Squad(int& step)
{
	static float movementSpeedY;
	static float movementSpeedZ;
	switch (step) {
	case 0:
		Idle();
		//modelAngles[RIGHT_LEG_UP].y = -20.0f;
		//modelAngles[LEFT_LEG_UP].y = 20.0f;
		modelAngles[LEFT_ARM_UP].y = -90.0f;
		modelAngles[RIGHT_ARM_UP].y = 90.0f;
		movementSpeedY = 0.0f;
		movementSpeedZ = 0.2f / tempActionSpeed;
		step = 1;
		break;
	case 1:

		//modelAngles[RIGHT_LEG_UP].y += 20.0f / actionSpeed;
		//modelAngles[LEFT_LEG_UP].y += -20.0f / actionSpeed;
		modelAngles[LEFT_LEG_UP].x += -55.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 45.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].x += -55.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 45.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += -45.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].x += -45.0f / tempActionSpeed;
		fullBodyMovement.y += -movementSpeedY;
		fullBodyMovement.z += -movementSpeedZ;
		fullBodyRotation.x += 10.0f / tempActionSpeed;
		movementSpeedY += 0.28f / tempActionSpeed / (2 * tempActionSpeed - 1);
		movementSpeedZ += -0.2f / tempActionSpeed / (2 * tempActionSpeed - 1);


		if (modelAngles[RIGHT_LEG_UP].x <= -110.0f)
			step = 2;
		break;
	case 2:
		//modelAngles[RIGHT_LEG_UP].y += -20.0f / actionSpeed;
		//modelAngles[LEFT_LEG_UP].y += 20.0f / actionSpeed;
		modelAngles[LEFT_LEG_UP].x += 55.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -45.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].x += 55.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -45.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += 45.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].x += 45.0f / tempActionSpeed;
		fullBodyMovement.y += movementSpeedY;
		fullBodyMovement.z += movementSpeedZ;
		fullBodyRotation.x += -10.0f / tempActionSpeed;

		movementSpeedY += -0.28 / tempActionSpeed / (2 * tempActionSpeed - 1);
		movementSpeedZ += 0.2 / tempActionSpeed / (2 * tempActionSpeed - 1);
		if (modelAngles[RIGHT_LEG_UP].x >= 0.0f) {
			movementSpeedY = 0.0f;
			movementSpeedZ = 0.2f / tempActionSpeed;
			step = 0;
		}

		break;
	}
	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

void Action::KamaHameHa(int& step)
{
	static int idleCount = 0;

	switch (step)
	{
	case 0:
		Idle();
		fullBodyRotation = glm::vec3(10.0, -85.0, 0.0);
		modelAngles[BODY_DOWN].z = 10.0;
		modelAngles[HEAD] = glm::vec3(-10.0, 80.0, 10.0);
		modelAngles[LEFT_ARM_UP] = glm::vec3(-160.0, -180.0, 160.0);
		modelAngles[LEFT_ARM_DOWN] = glm::vec3(-30.0, -5.0, -40.0);
		modelAngles[LEFT_HAND].z = 45.0;
		modelAngles[LEFT_LEG_UP] = glm::vec3(0.0, 5.0, 45.0);
		modelAngles[LEFT_FOOT] = glm::vec3(60.0, 25.0, 15.0);
		modelAngles[RIGHT_ARM_UP] = glm::vec3(5.0, 5.0, -80.0);
		modelAngles[RIGHT_ARM_DOWN] = glm::vec3(-55.0, -10.0, 75.0);
		modelAngles[RIGHT_LEG_UP] = glm::vec3(-30.0, -105.0, -50.0);
		modelAngles[RIGHT_LEG_DOWN] = glm::vec3(40.0, 90.0, -30.0);
		modelAngles[RIGHT_FOOT] = glm::vec3(10.0, -10.0, 0.0);
		fullBodyMovement.y = -0.12;

		step = 1;
		break;

	case 1:
		modelAngles[LEFT_LEG_UP].y += 90.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 35.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += 110.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += -60.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += -25.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -15.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].z += 35.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 5.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += -5.0f / tempActionSpeed;

		fullBodyMovement.y += 0.08f / tempActionSpeed;

		if (modelAngles[LEFT_LEG_UP].y >= 95.0)
			step = 2;
		break;
	case 2:
		fullBodyRotation.x += 15.0f / tempActionSpeed;
		fullBodyRotation.y += 85.0f / tempActionSpeed;

		modelAngles[BODY_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[HEAD].x += -15.0f / tempActionSpeed;
		modelAngles[HEAD].y += -80.0f / tempActionSpeed;
		modelAngles[HEAD].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 110.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += -150.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += -215.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += 30.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += -30.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 15.0f / tempActionSpeed;

		modelAngles[LEFT_HAND].z += 45.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -90.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += -95.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -80.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += -45.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -70.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -170.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -155.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 55.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -75.0f / tempActionSpeed;

		modelAngles[RIGHT_HAND].z += -90.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 105.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 15.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -90.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 25.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += 60.0f / tempActionSpeed;

		fullBodyMovement.y += -0.14f / tempActionSpeed;
		fullBodyMovement.z += 0.55f / tempActionSpeed;

		if (fullBodyRotation.x >= 25.0)
			step = 3;
		break;
	case 3:
		
		idleCount++;
		if (idleCount >= tempActionSpeed)
		{
			idleCount = 0;
			step = 0;
		}
		break;
	}

	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

void Action::BuuVictory(int& step)
{
	static int idleCount = 0;
	switch (step)
	{
	case 0:
		Idle();
		fullBodyRotation.y = 25.0;
		modelAngles[BODY_DOWN].y = -25.0;
		modelAngles[HEAD].y = 40.0;
		modelAngles[LEFT_ARM_UP] = glm::vec3(-60.0, 5.0, -25.0);
		modelAngles[LEFT_ARM_DOWN] = glm::vec3(0.0, 25.0, -55.0);
		modelAngles[LEFT_LEG_UP] = glm::vec3(0.0, 45.0, 25.0);
		modelAngles[LEFT_FOOT] = glm::vec3(20.0, 0.0, -15.0);
		modelAngles[RIGHT_ARM_UP] = glm::vec3(-25.0, -30.0, -105.0);
		modelAngles[RIGHT_ARM_DOWN] = glm::vec3(25.0, -20.0, 85.0);
		modelAngles[RIGHT_LEG_UP] = glm::vec3(-25.0, -60.0, -25.0);
		modelAngles[RIGHT_LEG_DOWN] = glm::vec3(40.0, 0.0, 0.0);
		modelAngles[RIGHT_FOOT] = glm::vec3(-5.0, -10.0, -5.0);
		fullBodyMovement.y = -0.04;

		step = 1;
		break;
	case 1:
		modelAngles[BODY_DOWN].y += 15.0f / tempActionSpeed;

		modelAngles[HEAD].y += -55.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 30.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 140.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += 25.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -35.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += 50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += -45.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 55.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 95.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -35.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -20.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 5.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += 55.0f / tempActionSpeed;

		fullBodyMovement.y += -0.02f / tempActionSpeed;

		if (modelAngles[RIGHT_LEG_UP].y >= 30.0)
			step++;
		break;
	case 2: case 10: case 18: case 26:
		fullBodyRotation.y += 20.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].y += 50.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 45.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -50.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 15.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 120.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].z += 5.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -65.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -130.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -30.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -45.0f / tempActionSpeed;

		if (modelAngles[LEFT_ARM_DOWN].z >= 30.0)
			step++;
		break;

	case 3: case 11: case 19:
		 fullBodyRotation.y += -20.0f / tempActionSpeed;

		modelAngles[BODY_DOWN].y += -15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 95.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += -70.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += 65.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -110.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -25.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 15.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 10.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += 105.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -75.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -50.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 60.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 10.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += -55.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].y += 15.0f / tempActionSpeed;

		if (modelAngles[LEFT_ARM_DOWN].z <= -80.0)
			step++;
		break;

	case 4: case 12: case 20:
		fullBodyRotation.y += -50.0f / tempActionSpeed;

		modelAngles[BODY_DOWN].y += 50.0f / tempActionSpeed;

		modelAngles[HEAD].y += 30.0f / tempActionSpeed;

		if (modelAngles[BODY_DOWN].y >= 25.0)
			step++;
		break;

	case 5: case 13: case 21:
		modelAngles[BODY_DOWN].y += -15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 155.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += -80.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -50.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 10.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 15.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += 30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += -60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += 5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 55.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 30.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -25.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -20.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 145.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -25.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -25.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 10.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 15.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += -20.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += 15.0f / tempActionSpeed;

		if (modelAngles[LEFT_ARM_UP].z <= 10.0)
			step++;
		break;
	case 6: case 14: case 22:
		modelAngles[LEFT_ARM_UP].x += -65.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 130.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 30.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -10.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += -15.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 45.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].y += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -45.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -120.0f / tempActionSpeed;
		if (modelAngles[RIGHT_ARM_DOWN].z <= -30.0)
			step++;
		break;
	case 7: case 15: case 23:
		modelAngles[BODY_DOWN].y += 15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 105.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 75.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 50.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -60.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += 60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 10.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += -5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += -55.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += -30.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 25.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -95.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 70.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 65.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 110.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += -25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -15.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += -15.0f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_DOWN].z >= 80.0)
			step++;
		break;
	case 8: case 16: case 24:
		fullBodyRotation.y += 50.0f / tempActionSpeed;

		modelAngles[BODY_DOWN].y += -50.0f / tempActionSpeed;

		modelAngles[HEAD].y += -30.0f / tempActionSpeed;
		if (fullBodyRotation.y >= 25.0)
			step++;
		break;
	case 9: case 17: case 25:
		modelAngles[BODY_DOWN].y += 15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += -145.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 25.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -15.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 25.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += 25.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += -5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -15.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += -5.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -15.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -40.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -155.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 80.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -15.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 10.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += 5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += 55.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].y += -15.0f / tempActionSpeed;

		if (modelAngles[BODY_DOWN].y >= -10.0)
			step++;
		break;
	case 27:
		fullBodyRotation.x += 10.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 155.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 135.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -15.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 10.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 30.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += 5.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 5.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].y += -100.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -120.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 15.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 15.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += -110.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -80.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 20.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].y += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += -5.0f / tempActionSpeed;

		fullBodyMovement.y += 0.11f / tempActionSpeed;

		if (modelAngles[RIGHT_LEG_UP].y <= -80.0f)
			step++;
		break;
	case 28:
		fullBodyRotation.y += -20.0f / tempActionSpeed;
		fullBodyRotation.z += -40.0f / tempActionSpeed;

		modelAngles[HEAD].x += -20.0f / tempActionSpeed;
		modelAngles[HEAD].y += 15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += 70.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 65.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -70.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 30.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += -10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 20.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -35.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -75.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 15.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 30.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 30.0f / tempActionSpeed;

		fullBodyMovement.y += 0.03f / tempActionSpeed;

		if (fullBodyRotation.y <= 25.0)
			step++;
		break;
	case 29:
		fullBodyRotation.x += -10.0f / tempActionSpeed;
		fullBodyRotation.z += 40.0f / tempActionSpeed;

		modelAngles[BODY_DOWN].y += -15.0f / tempActionSpeed;

		modelAngles[HEAD].x += 20.0f / tempActionSpeed;
		modelAngles[HEAD].y += -15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += -150.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 90.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += 100.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += 10.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -70.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].y += -5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -15.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += 140.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 145.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 15.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += -40.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 40.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += 5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += -25.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += 5.0f / tempActionSpeed;

		fullBodyMovement.y += -0.14f / tempActionSpeed;

		if (modelAngles[RIGHT_LEG_UP].y >= -30.0)
			step++;
		break;
	case 30:
		modelAngles[BODY_DOWN].y += 15.0f / tempActionSpeed;

		modelAngles[HEAD].x += 15.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -80.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += -75.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += -10.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].x += -40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 25.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].y += 5.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += 15.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += -50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -40.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 10.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 5.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += -10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += 10.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += -85.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 110.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -20.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += -100.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -20.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += -10.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += 10.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 5.0f / tempActionSpeed;

		fullBodyMovement.y += -0.04f / tempActionSpeed;

		if (modelAngles[BODY_DOWN].y >= -10.0)
			step++;
		break;
	case 31:
		modelAngles[HEAD].x += -25.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 85.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].y += -25.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_UP].z += 80.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_DOWN].y += 40.0f / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 55.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += 15.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].y += -10.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].z += -25.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_DOWN].x += 25.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 35.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 5.0f / tempActionSpeed;

		modelAngles[LEFT_FOOT].x += 10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].y += 10.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].z += -5.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_UP].x += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -10.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -50.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -60.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].x += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].y += 15.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].z += 25.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_DOWN].x += -55.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += 5.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += -20.0f / tempActionSpeed;

		modelAngles[RIGHT_FOOT].x += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].y += 25.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].z += -5.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[HEAD].x <= -10.0)
			step++;
		break;

	case 32:
		
		idleCount++;
		if (idleCount >= tempActionSpeed)
		{
			idleCount = 0;
			step = 0;
		}
		break;
	}
	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

void Action::Moonwalk(int& step)
{
	switch (step)
	{
	case 0:
		ResetRotation();
		modelAngles[RIGHT_LEG_UP].x = 20.0f;
		modelAngles[RIGHT_LEG_DOWN].x = 40.0f;
		modelAngles[RIGHT_FOOT].x = -25.0f;
		modelAngles[LEFT_ARM_UP].x = -20.0f;
		modelAngles[LEFT_ARM_DOWN].x = -40.0f;
		modelAngles[RIGHT_ARM_UP].x = 20.0f;
		modelAngles[RIGHT_ARM_DOWN].x = -40.0f;
		step = 1;
		break;

	case 1:
		modelAngles[RIGHT_LEG_UP].x += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].x += 35.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 40.0f * 0.67f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += -40.0f * 0.67f / tempActionSpeed;

		fullBodyMovement.z += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_LEG_UP].x <= -40.0f)
			step = 2;
		break;

	case 2:
		modelAngles[RIGHT_LEG_UP].x += 40.0f * 2.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -40.0f * 2.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].x += -10.0f * 2.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_UP].x += 20.0f * 2.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 40.0f * 2.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].x += -25.0f * 2.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 40.0f * 0.33f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += -40.0f * 0.33f / tempActionSpeed;

		fullBodyMovement.z += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_LEG_UP].x >= 0.0f)
			step = 3;
		break;

	case 3:
		modelAngles[LEFT_LEG_UP].x += -60.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].x += 35.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -40.0f * 0.67f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += 40.0f * 0.67f / tempActionSpeed;

		fullBodyMovement.z += -0.3f / tempActionSpeed;

		if (modelAngles[LEFT_LEG_UP].x <= -40.0f)
			step = 4;
		break;

	case 4:
		modelAngles[LEFT_LEG_UP].x += 40.0f * 2.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -40.0f * 2.0f / tempActionSpeed;
		modelAngles[LEFT_FOOT].x += -10.0f * 2.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_UP].x += 20.0f * 2.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 40.0f * 2.0f / tempActionSpeed;
		modelAngles[RIGHT_FOOT].x += -25.0f * 2.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -40.0f * 0.33f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].x += 40.0f * 0.33f / tempActionSpeed;

		fullBodyMovement.z += -0.3f / tempActionSpeed;

		if (modelAngles[LEFT_LEG_UP].x >= 0.0f)
			step = 0;
		break;
	}
	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];

	if (fullBodyMovement.z <= -4.0f)
		fullBodyMovement.z = 0;
}

void Action::GangnamSytle(int& step)
{

	static float movementSpeedY;

	switch (step)
	{
	case 0:
		ResetRotation();

		modelAngles[RIGHT_ARM_UP] = glm::vec3(0.0f, -90.0f, -90.0f);
		modelAngles[RIGHT_ARM_DOWN] = glm::vec3(-55.0f, -90.0f, 0.0f);

		modelAngles[LEFT_ARM_UP].x = -75.0f;
		modelAngles[LEFT_ARM_DOWN] = glm::vec3(0.0f, -90.0f, -50.0f);

		modelAngles[RIGHT_LEG_UP] = glm::vec3(0.0f, -50.0f, -30.0f);
		modelAngles[RIGHT_LEG_DOWN].x = 30.0f;
		modelAngles[RIGHT_FOOT] = glm::vec3(-5.0f, 0.0f, 20.0f);

		modelAngles[LEFT_LEG_UP] = glm::vec3(0.0f, 50.0f, 30.0f);
		modelAngles[LEFT_LEG_DOWN].x = 30.0f;
		modelAngles[LEFT_FOOT] = glm::vec3(-5.0f, 0.0f, -20.0f);

		fullBodyMovement.y = -0.02f;

		movementSpeedY = 0.1 / tempActionSpeed;

		step = 1;
		break;

	case 1: case 5: case 7: case 11:
		modelAngles[RIGHT_ARM_UP].x += -30.0f / tempActionSpeed;

		modelAngles[RIGHT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 105.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 65.0f / tempActionSpeed;


		modelAngles[RIGHT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -30.0f)
			step += 1;
		break;

	case 2: case 6: case 8: case 12:
		modelAngles[RIGHT_ARM_UP].x += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].x += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 255.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 295.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].z += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -15.0f / tempActionSpeed;

		if (fullBodyMovement.y >= 0)
			fullBodyMovement.y += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= 0.0f)
			step += 1;
		break;
	case 3: case 9: case 13: case 15:
		modelAngles[RIGHT_ARM_UP].x += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].x += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 105.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 65.0f / tempActionSpeed;


		modelAngles[LEFT_LEG_UP].z += 30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += -50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -30.0f)
			step += 1;
		break;
	case 4: case 10: case 14: case 16:
		modelAngles[RIGHT_ARM_UP].x += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].x += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 255.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 295.0f / tempActionSpeed;


		modelAngles[LEFT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -15.0f / tempActionSpeed;

		if (fullBodyMovement.y >= 0)
			fullBodyMovement.y += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= 0.0f)
			step += 1;
		break;
	case 17:
		modelAngles[RIGHT_ARM_UP].x += -95.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += 90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].x += 55.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += 180.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 50.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -95.0f)
			step += 1;
		break;

	case 21: case 23: case 27:
		modelAngles[RIGHT_ARM_UP].x += -30.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -30.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -95.0f)
			step += 1;
		break;

	case 18: case 22: case 24: case 28:
		modelAngles[RIGHT_ARM_UP].x += 30.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 30.0f / tempActionSpeed;

		modelAngles[RIGHT_LEG_UP].z += 30.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].y += 60.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].z += -50.0f / tempActionSpeed;
		modelAngles[RIGHT_LEG_DOWN].x += -15.0f / tempActionSpeed;

		if (fullBodyMovement.y >= 0)
			fullBodyMovement.y += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= -65.0f)
			step += 1;
		break;

	case 19: case 25: case 29: case 31:
		modelAngles[RIGHT_ARM_UP].x += -30.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -30.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].z += 30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += 60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += -50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += 15.0f / tempActionSpeed;

		fullBodyMovement.y += 0.1f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -95.0f)
			step += 1;
		break;

	case 20: case 26: case 30:
		modelAngles[RIGHT_ARM_UP].x += 30.0f / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 30.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -15.0f / tempActionSpeed;

		if (fullBodyMovement.y >= 0)
			fullBodyMovement.y += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= -65.0f)
			step += 1;
		break;

	case 32:

		modelAngles[RIGHT_ARM_UP].x += 95.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].y += -90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_UP].z += -90.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].x += -55.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].y += -180.0f / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -50.0f / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].z += -30.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -20.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].y += -60.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].z += 50.0f / tempActionSpeed;
		modelAngles[LEFT_LEG_DOWN].x += -15.0f / tempActionSpeed;

		if (fullBodyMovement.y >= 0)
			fullBodyMovement.y += -0.3f / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= 0.0f)
			step = 0;
		break;

	default:
		break;
	}

	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

void Action::SitUp(int& step)
{
	switch (step)
	{
	case 0:
		ResetRotation();
		modelAngles[RIGHT_LEG_UP].x = -70.0f;
		modelAngles[RIGHT_LEG_DOWN].x = 120.0f;
		modelAngles[RIGHT_FOOT].x = 40.0f;

		modelAngles[LEFT_LEG_UP].x = -70.0f;
		modelAngles[LEFT_LEG_DOWN].x = 120.0f;
		modelAngles[LEFT_FOOT].x = 40.0f;

		modelAngles[RIGHT_ARM_UP].x = -20.0f;
		modelAngles[RIGHT_ARM_UP].y = 30.0f;
		modelAngles[RIGHT_ARM_DOWN].x = -140.0f;

		modelAngles[LEFT_ARM_UP].x = -20.0f;
		modelAngles[LEFT_ARM_UP].y = -30.0f;
		modelAngles[LEFT_ARM_DOWN].x = -140.0f;

		fullBodyMovement.y = -0.74f;
		fullBodyRotation.x = -90.0f;

		step = 1;
		break;

	case 1:
		modelAngles[RIGHT_LEG_UP].x += -45.0f / 2 / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += -45.0f / 2 / tempActionSpeed;

		fullBodyMovement.y += 0.12f / 2 / tempActionSpeed;
		fullBodyMovement.z += 0.08f / 2 / tempActionSpeed;
		fullBodyRotation.x += 45.0f / 2 / tempActionSpeed;
		if (modelAngles[RIGHT_LEG_UP].x <= -115.0f)
			step = 2;
		break;

	case 2:
		modelAngles[RIGHT_LEG_UP].x += 45.0f / 2 / tempActionSpeed;

		modelAngles[LEFT_LEG_UP].x += 45.0f / 2 / tempActionSpeed;

		fullBodyMovement.y += -0.12f / 2 / tempActionSpeed;
		fullBodyMovement.z += -0.08f / 2 / tempActionSpeed;
		fullBodyRotation.x += -45.0f / 2 / tempActionSpeed;
		if (modelAngles[RIGHT_LEG_UP].x >= -70.0f)
			step = 0;
		break;

	default:
		break;
	}

	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

void Action::PushUp(int& step)
{
	switch (step)
	{
	case 0:
		ResetRotation();
		modelAngles[RIGHT_ARM_UP] = glm::vec3(-50.0f, -90.0f, -90.0f);
		modelAngles[RIGHT_ARM_DOWN] = glm::vec3(0.0f, 90.0f, 40.0f);

		modelAngles[LEFT_ARM_UP] = glm::vec3(-50.0f, 90.0f, 90.0f);
		modelAngles[LEFT_ARM_DOWN] = glm::vec3(0.0f, -90.0f, -40.0f);

		fullBodyRotation.x = 87.0f;
		fullBodyMovement = glm::vec3(0.0f, -0.53f, 0.5f);
		step = 1;
		break;

	case 1:
		modelAngles[RIGHT_ARM_UP].x += 50.0f / 2 / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += 50.0f / 2 / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += 50.0f / 2 / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += -50.0f / 2 / tempActionSpeed;

		fullBodyRotation.x += 6.0f / 2 / tempActionSpeed;
		fullBodyMovement.y += -0.08f / 2 / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x >= 0.0f)
			step = 2;
		break;

	case 2:
		modelAngles[RIGHT_ARM_UP].x += -50.0f / 2 / tempActionSpeed;
		modelAngles[RIGHT_ARM_DOWN].z += -50.0f / 2 / tempActionSpeed;

		modelAngles[LEFT_ARM_UP].x += -50.0f / 2 / tempActionSpeed;
		modelAngles[LEFT_ARM_DOWN].z += 50.0f / 2 / tempActionSpeed;

		fullBodyRotation.x += -6.0f / 2 / tempActionSpeed;
		fullBodyMovement.y += 0.08f / 2 / tempActionSpeed;

		if (modelAngles[RIGHT_ARM_UP].x <= -50.0f)
			step = 0;
		break;


	default:
		break;
	}

	modelAngles[LEFT_SHOULDER] = modelAngles[LEFT_ARM_UP];
	modelAngles[RIGHT_SHOULDER] = modelAngles[RIGHT_ARM_UP];
}

