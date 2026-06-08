#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
// Custom headers
#include "parser.h"
#include "shader.h"
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

    // sjader
    Shader nodeShader(
        "../../../shaders/node.vert", 
        "../../../shaders/node.frag"
    );

	float quadVertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		 -1.0f,  1.0f
	};

	unsigned int quadIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

    struct NodeInstance {
        glm::vec2 position;
        glm::vec3 color;
    };

    std::vector<NodeInstance> nodeInstances;
    for (auto& n : cap.nodes)
        nodeInstances.push_back({ n.position, glm::vec3(0.3f, 0.7f, 1.0f) });

    // buffer setup
	unsigned int quadVAO, quadVBO, quadEBO, instanceVBO;

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);
	glGenBuffers(1, &instanceVBO);

	glBindVertexArray(quadVAO);

    // quad vertices
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
    
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    
    // per instance
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, nodeInstances.size() * sizeof(NodeInstance), nodeInstances.data(), GL_STATIC_DRAW);

    // location 1 offset
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(NodeInstance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // llocation 2 color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(NodeInstance), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);

    // blending for smooth circle edges (idk how it works)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // edge rendering

	Shader edgeShader(
		"../../../shaders/edge.vert",
		"../../../shaders/edge.frag"
	);

	std::vector<glm::vec2> edgeVertices;
	for (auto& e : cap.edges)
	{
		edgeVertices.push_back(cap.nodes[e.src].position);
		edgeVertices.push_back(cap.nodes[e.dst].position);
	}
    // buffer setup
    unsigned int edgeVAO, edgeVBO;
    glGenVertexArrays(1, &edgeVAO);
    glGenBuffers(1, &edgeVBO);

    glBindVertexArray(edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, edgeVertices.size() * sizeof(glm::vec2), edgeVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // packet animation
	Shader packetShader(
		"../../../shaders/packet.vert",
		"../../../shaders/packet.frag"
	);

    unsigned int packetInstanceVBO;
	glGenBuffers(1, &packetInstanceVBO);

	float simTime = 0.0f;
	float simSpeed = 1.0f;
	const float TRAVEL_TIME = 2.0f;

    std::vector<AnimatedPacket> activePackets;
	int nextEventIndex = 0;


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // delta time

        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        simTime += deltaTime * simSpeed;

        // Clear screen
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
		edgeShader.use();
		edgeShader.setVec3("uColor", glm::vec3(0.3f, 0.5f, 0.7f));
		edgeShader.setFloat("uAlpha", 0.6f);

		glBindVertexArray(edgeVAO);
		glDrawArrays(GL_LINES, 0, (int)edgeVertices.size());


        // animation

		while (nextEventIndex < (int)cap.events.size() && cap.events[nextEventIndex].timestamp <= simTime)
		{
            AnimatedPacket ap;
            ap.eventID = nextEventIndex;
			ap.progress = 0.0f;
			activePackets.push_back(ap);
			nextEventIndex++;
		}

        for (auto& ap : activePackets)
        {
            ap.progress = (simTime - cap.events[ap.eventID].timestamp) / TRAVEL_TIME;
        }

		activePackets.erase(std::remove_if(activePackets.begin(), activePackets.end(),
			[](const AnimatedPacket& ap) { return ap.progress >= 1.0f; }),
			activePackets.end()
		); // gpt fix

        struct PacketInstance {
            glm::vec2 position;
            glm::vec3 color;
        };
        
        auto protoColor = [](Protocol p) -> glm::vec3 {
            switch (p) {
			case Protocol::TCP:   return { 0.2f, 0.6f, 1.0f }; // blue
            case Protocol::UDP:   return { 0.4f, 1.0f, 0.4f }; // green
            case Protocol::ICMP:  return { 1.0f, 0.4f, 0.4f }; // red
            case Protocol::DNS:   return { 1.0f, 0.8f, 0.2f }; // orange
            default:              return { 0.8f, 0.8f, 0.8f }; // gray
            }
         };

        std::vector<PacketInstance> packetInstances;
        for (auto& ap : activePackets)
        {
            auto& ev = cap.events[ap.eventID];
            glm::vec2 src = cap.nodes[ev.src].position;
            glm::vec2 dst = cap.nodes[ev.dst].position;
            glm::vec2 pos = src + (dst - src) * ap.progress;

            packetInstances.push_back({ pos, protoColor(ev.protocol) });
        }

        glBindBuffer(GL_ARRAY_BUFFER, packetInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
            packetInstances.size() * sizeof(PacketInstance),
            packetInstances.data(), GL_DYNAMIC_DRAW);

        if (!packetInstances.empty())
        {
            glBindVertexArray(quadVAO);

            // rebind instance VBO with packet data
            glBindBuffer(GL_ARRAY_BUFFER, packetInstanceVBO);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PacketInstance), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribDivisor(1, 1);

            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PacketInstance), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);

            packetShader.use();
            packetShader.setFloat("uRadius", 8.0f);
            packetShader.setVec2("uResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (int)packetInstances.size());

            // restore node instance VBO for next frame
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(NodeInstance), (void*)0);
            glVertexAttribDivisor(1, 1);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(NodeInstance), (void*)(2 * sizeof(float)));
            glVertexAttribDivisor(2, 1);

            glBindVertexArray(0);
        }

        nodeShader.use();
        nodeShader.setFloat("uRadius", 18.0f);
        nodeShader.setVec2("uResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

        glBindVertexArray(quadVAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (int)nodeInstances.size());



        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();



    return 0;
}