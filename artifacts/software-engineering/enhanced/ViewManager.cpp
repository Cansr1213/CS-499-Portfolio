// CS-499 Milestone Two Enhancement:
// Added documentation, improved consistency, and added defensive checks.

///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// Camera object used for viewing and interacting with the 3D scene
	Camera* g_pCamera = nullptr;

	// Mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// Frame timing
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// Projection mode flag
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;

	g_pCamera = new Camera();

	// Default camera parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.5f, 8.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
	g_pCamera->MovementSpeed = 20;
}

/***********************************************************
 *  ~ViewManager()
 ***********************************************************/
ViewManager::~ViewManager()
{
	m_pShaderManager = NULL;
	m_pWindow = NULL;

	if (g_pCamera != NULL)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	// Mouse callbacks
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Wheel_Callback);

	// Capture mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;
	return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos;

	gLastX = xMousePos;
	gLastY = yMousePos;

	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  Mouse_Scroll_Wheel_Callback()
 ***********************************************************/
void ViewManager::Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double x, double yScrollDistance)
{
	g_pCamera->ProcessMouseScroll(yScrollDistance);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// CS-499 Enhancement: Defensive check
	if (m_pWindow == nullptr)
	{
		std::cerr << "ERROR: Window pointer is null in ProcessKeyboardEvents()." << std::endl;
		return;
	}

	if (g_pCamera == NULL)
	{
		return;
	}

	// Close window
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_pWindow, true);

	// Movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);

	// CS-499 Enhancement: Documented projection modes
	// 1 = Front orthographic  
	// 2 = Side orthographic  
	// 3 = Top orthographic  
	// 4 = Perspective view

	if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::vec3(0.0f, 4.0f, 10.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::vec3(10.0f, 4.0f, 0.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Front = glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_3) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::vec3(0.0f, 7.0f, 0.0f);
		g_pCamera->Up = glm::vec3(-1.0f, 0.0f, 0.0f);
		g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_4) == GLFW_PRESS)
	{
		bOrthographicProjection = false;
		g_pCamera->Position = glm::vec3(0.0f, 5.5f, 8.0f);
		g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

/***********************************************************
 *  PrepareSceneView()
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// CS-499 Enhancement: Defensive programming checks
	if (m_pShaderManager == nullptr)
	{
		std::cerr << "ERROR: ShaderManager is null in PrepareSceneView()." << std::endl;
		return;
	}
	if (g_pCamera == nullptr)
	{
		std::cerr << "ERROR: Camera pointer is null in PrepareSceneView()." << std::endl;
		return;
	}

	// Frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	ProcessKeyboardEvents();

	// Camera view matrix
	view = g_pCamera->GetViewMatrix();

	// CS-499 Enhancement: Clear projection documentation
	// Perspective = realistic depth for normal interaction
	// Orthographic = distortion-free inspection views

	if (!bOrthographicProjection)
	{
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,
			0.1f,
			100.0f
		);
	}
	else
	{
		double scale = 0.0;

		if (WINDOW_WIDTH > WINDOW_HEIGHT)
		{
			scale = (double)WINDOW_HEIGHT / (double)WINDOW_WIDTH;
			projection = glm::ortho(-5.0f, 5.0f, -5.0f * (float)scale, 5.0f * (float)scale, 0.1f, 100.0f);
		}
		else if (WINDOW_WIDTH < WINDOW_HEIGHT)
		{
			scale = (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT;
			projection = glm::ortho(-5.0f * (float)scale, 5.0f * (float)scale, -5.0f, 5.0f, 0.1f, 100.0f);
		}
		else
		{
			projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
		}
	}

	// Load view/projection matrices to shader
	m_pShaderManager->setMat4Value(g_ViewName, view);
	m_pShaderManager->setMat4Value(g_ProjectionName, projection);

	// Update lighting with camera position & direction
	m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	m_pShaderManager->setVec3Value("spotLight.position", g_pCamera->Position);
	m_pShaderManager->setVec3Value("spotLight.direction", g_pCamera->Front);
}

