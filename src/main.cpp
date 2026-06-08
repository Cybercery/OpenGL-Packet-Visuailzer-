#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
// Custom headers
#include "parser.h"

// Window settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{

    ParsedCapture cap = parseCapture("../../../data/capture.json");

    std::cout << "\nNodes\n";
    for (auto& n : cap.nodes)
        std::cout << "[" << n.id << "]" << n.ip
        << "pkts=" << n.totalPackets
        << "bytes=" << n.totalBytes << "\n";

    std::cout << "\nEdges\n";
    for (auto& e : cap.edges)
        std::cout << cap.nodes[e.src].ip
        << "<->" << cap.nodes[e.dst].ip
        << " pkts=" << e.packetCount
        << " bytes=" << e.bytesTransferred << "\n";

    // Initialize GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Network Visualizer",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Enable VSync
    glfwSwapInterval(1);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    // Initial viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Clear screen
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw here

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();



    return 0;
}