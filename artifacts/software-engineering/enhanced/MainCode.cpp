///////////////////////////////////////////////////////////////////////////////
// maincode.cpp
// ============
// gets called when application is launched - initializes GLEW, GLFW
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//
//  CS-499 Milestone Two Enhancement:
//  Added defensive initialization checks, documentation updates,
//  and improved reliability for portfolio-quality code.
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    const char* const WINDOW_TITLE = "5-2 Assignment";

    GLFWwindow* g_Window = nullptr;

    SceneManager* g_SceneManager = nullptr;
    ShaderManager* g_ShaderManager = nullptr;
    ViewManager* g_ViewManager = nullptr;
}

bool InitializeGLFW();
bool InitializeGLEW();

/***********************************************************
 *  main(int, char*)
 ***********************************************************/
int main(int argc, char* argv[])
{
    // Defensive check: GLFW initialization
    if (!InitializeGLFW())
    {
        std::cerr << "ERROR: Failed to initialize GLFW." << std::endl;
        return EXIT_FAILURE;
    }

    g_ShaderManager = new ShaderManager();
    g_ViewManager = new ViewManager(g_ShaderManager);

    // Create main window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

    // NEW CHECK — ensure window was actually created
    if (!g_Window)
    {
        std::cerr << "ERROR: Failed to create GLFW window." << std::endl;
        return EXIT_FAILURE;
    }

    // Defensive check: GLEW initialization
    if (!InitializeGLEW())
    {
        std::cerr << "ERROR: Failed to initialize GLEW." << std::endl;
        return EXIT_FAILURE;
    }

    // Load shader files
    g_ShaderManager->LoadShaders(
        "shaders/vertexShader.glsl",
        "shaders/fragmentShader.glsl"
    );
    g_ShaderManager->use();

    // Load 3D scene
    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    std::cout << "\n*** KEY FUNCTIONS: ***\n";
    std::cout << "ESC - close the window and exit\n";
    std::cout << "W - zoom in\tS - zoom out\n";
    std::cout << "A - pan left\tD - pan right\n";
    std::cout << "Q - pan up\tE - pan down\n";
    std::cout << "1 - front view (ortho)\n";
    std::cout << "2 - side view (ortho)\n";
    std::cout << "3 - top view (ortho)\n";
    std::cout << "4 - perspective view\n";

    while (!glfwWindowShouldClose(g_Window))
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_ViewManager->PrepareSceneView();
        g_SceneManager->RenderScene();

        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    if (g_SceneManager) { delete g_SceneManager; g_SceneManager = NULL; }
    if (g_ViewManager) { delete g_ViewManager;  g_ViewManager = NULL; }
    if (g_ShaderManager) { delete g_ShaderManager; g_ShaderManager = NULL; }

    exit(EXIT_SUCCESS);
}

/***********************************************************
 *  InitializeGLFW()
 ***********************************************************/
bool InitializeGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "ERROR: GLFW initialization failed." << std::endl;
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

/***********************************************************
 *  InitializeGLEW()
 ***********************************************************/
bool InitializeGLEW()
{
    GLenum GLEWInitResult = glewInit();

    if (GLEWInitResult != GLEW_OK)
    {
        std::cerr << "GLEW ERROR: "
            << glewGetErrorString(GLEWInitResult)
            << std::endl;
        return false;
    }

    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n\n";

    return true;
}
