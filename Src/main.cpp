/*
GDGRAP1 Final Project: Mari of Kart Speed Unwanted '25
Main Game Implementation File

This file contains the core game loop, rendering logic, and input handling
for a simple racing game with player and ghost karts.
*/

// Include necessary libraries
#include <glad/glad.h>          // OpenGL function pointers
#include <GLFW/glfw3.h>         // Window and input handling
#include <glm/glm.hpp>          // Math library for vectors/matrices
#include <glm/gtc/matrix_transform.hpp> // Matrix transformations
#include <glm/gtc/type_ptr.hpp> // Type conversion utilities
#include <iostream>             // Console output
#include <vector>               // Dynamic arrays
#include <fstream>              // File operations
#include <sstream>              // String stream operations
#include <algorithm>            // Sorting algorithms
#include "Shader.h"             // Custom shader wrapper class
#include "Light.h"              // Custom light class
#include "Camera.h"             // Custom camera class

// Image loading library implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// =============================================
// Global Constants and Variables
// =============================================

// Skybox modes (day/night)
enum SkyboxMode { DAY, NIGHT };
SkyboxMode currentSkybox = DAY;

// Skybox texture paths for day and night
std::vector<std::string> dayFaces = {
    "skybox/right1.png", "skybox/left1.png",
    "skybox/top1.png",   "skybox/bottom1.png",
    "skybox/front1.png", "skybox/back1.png"
};

std::vector<std::string> nightFaces = {
    "skybox/right2.png", "skybox/left2.png",
    "skybox/top2.png",   "skybox/bottom2.png",
    "skybox/front2.png", "skybox/back2.png"
};

// Skybox vertex data (cube)
float skyboxVertices[] = {
    // Positions for a cube (36 vertices)     
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
};

// Global light source (directional light)
Light directionalLight(
    glm::vec3(-0.5f, -1.0f, -0.5f), // Direction
    glm::vec3(0.3f, 0.3f, 0.3f),    // Ambient
    glm::vec3(0.8f, 0.8f, 0.7f),    // Diffuse
    glm::vec3(0.5f, 0.5f, 0.5f),    // Specular
    1.0f                            // Intensity
);

// Ground plane vertices (position, normal, texture coords)
float planeVertices[] = {
    
     25.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -25.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -25.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

     25.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -25.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     25.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f
};

// Vertex structure for 3D models
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

// Texture structure for materials
struct Texture {
    unsigned int id;    // OpenGL texture ID
    std::string type;   // "texture_diffuse" or "texture_specular"
    std::string path;   // File path
};

// Mesh class for rendering 3D models
class Mesh {
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // OpenGL buffers
    unsigned int VAO, VBO, EBO;

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        setupMesh();
    }

    // Draw the mesh with a shader
    void Draw(Shader& shader, float alpha = 1.0f) {
        // Bind textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;

        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);

            // Determine texture type (diffuse/specular)
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);

            // Set shader uniform and bind texture
            shader.setInt((name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // Set transparency value
        shader.setFloat("material.alpha", alpha);

        // Draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    // Initialize OpenGL buffers for the mesh
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // Element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Vertex attributes
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);
    }
};
// Initialize the camera at a specific position (x=0, y=1, z=3)
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));

// Variables for mouse movement tracking
float lastX = 400, lastY = 300;
bool firstMouse = true;

// Timing variables for frame update consistency
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Game finish line and timing
const float FINISH_LINE_Z = 40.0f;  // Z position of the finish line
bool gameFinished = false;         // Flag to check if race has ended
float raceStartTime = 0.0f;        // Time when race started
float finishTime = 0.0f;           // Time when player finished

// Flags for each kart's finish state
bool playerFinished = false;
bool ghost1Finished = false;
bool ghost2Finished = false;

// Function declarations for utilities
unsigned int loadCubemap(std::vector<std::string> faces);       // Loads skybox textures
unsigned int loadTexture(const char* path);                     // Loads a single texture
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Resizes window viewport
void mouse_callback(GLFWwindow* window, double xpos, double ypos);         // Handles mouse movement
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Handles mouse scroll (zoom)
void checkTextureLoading(const std::vector<std::string>& faces);          // Debug texture loading
bool loadOBJ(const std::string& path, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices); // Loads .obj models
std::vector<std::string> split(const std::string& s, char delim);         // Utility to split strings

// Player kart properties
glm::vec3 kartPosition(0.0f, 0.05f, -48.0f); // Starting position of the player kart
float kartRotation = 0.0f;                  // Current rotation of the player kart
float kartSpeed = 0.0f;                     // Current speed of the player kart
const float MAX_SPEED = 9.0f;               // Maximum forward speed of the player kart
const float ACCELERATION = 4.5f;            // Forward acceleration rate
const float TURN_SPEED = 1.5f;              // Rotation speed of the kart

// Ghost kart 1 (side kart) properties
float sideKartSpeed = 0.0f;
const float SIDE_KART_MAX_SPEED = 15.0f;    // Max speed of ghost kart 1
const float SIDE_KART_ACCELERATION = 6.0f;  // Acceleration of ghost kart 1
bool spacePressed = false;                 // Tracks spacebar press
bool ghostKartsMoving = false;             // Whether ghost karts are moving

// Ghost kart 2 (side kart) properties
float sideKart2Speed = 0.0f;
const float SIDE_KART2_MAX_SPEED = 6.0f;    // Max speed of ghost kart 2
const float SIDE_KART2_ACCELERATION = 1.0f; // Acceleration of ghost kart 2

// Starting positions of the ghost karts
glm::vec3 ghostKart1Position(0.0f, 0.05f, -48.0f);
glm::vec3 ghostKart2Position(0.0f, 0.05f, -48.0f);

// Ghost kart visuals and spacing
const float SIDE_KART_DISTANCE = 3.0f;      // Horizontal distance from player kart
const float SIDE_KART_ALPHA = 0.7f;         // Transparency of ghost karts

// Landmark placement settings
const float LANDMARK_DISTANCE_FROM_FINISH = 5.0f; // Distance from finish line to landmark
const float LANDMARK_SPACING = 15.0f;             // Distance between each landmark

// Rotation of side (ghost) karts
float sideKartRotation = 0.0f;

// Camera mode tracking
CameraMode cameraMode = THIRD_PERSON;  // Default to third-person view
bool zPressed = false;                 // Tracks Z key press (camera toggle)


int main() {
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Mario of Kart Speed Unwanted ‘25 by Anton Teodoro", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader groundShader("ground.vert", "ground.frag");
    Shader kartShader("kart.vert", "kart.frag");

    GLint success;
    char infoLog[512];

    glGetProgramiv(skyboxShader.ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(skyboxShader.ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::SKYBOX::LINKING_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    glGetProgramiv(groundShader.ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(groundShader.ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::GROUND::LINKING_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    glGetProgramiv(kartShader.ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(kartShader.ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::KART::LINKING_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    checkTextureLoading(dayFaces);
    checkTextureLoading(nightFaces);

    unsigned int dayCubemap = loadCubemap(dayFaces);
    unsigned int nightCubemap = loadCubemap(nightFaces);
    if (dayCubemap == 0 || nightCubemap == 0) {
        std::cerr << "Failed to load cubemap textures!" << std::endl;
        return -1;
    }

    unsigned int groundTexture = loadTexture("assets/ground.jpg");
    if (groundTexture == 0) {
        std::cerr << "Failed to load ground texture!" << std::endl;
        return -1;
    }

    unsigned int finishLineTexture = loadTexture("assets/finish_line.png");

    std::vector<Vertex> landmark1Vertices, landmark2Vertices;
    std::vector<unsigned int> landmark1Indices, landmark2Indices;
    if (!loadOBJ("assets/kart.obj", landmark1Vertices, landmark1Indices) ||
        !loadOBJ("assets/kart.obj", landmark2Vertices, landmark2Indices)) {
        std::cerr << "Failed to load landmark models!" << std::endl;
        return -1;
    }

    std::vector<Texture> landmark1Textures, landmark2Textures;
    Texture landmark1Texture;
    landmark1Texture.id = loadTexture("assets/Landmark_1.png");
    if (landmark1Texture.id == 0) {
        std::cerr << "Failed to load Landmark1 texture!" << std::endl;
        return -1;
    }
    landmark1Texture.type = "texture_diffuse";
    landmark1Texture.path = "assets/Landmark_1.png";
    landmark1Textures.push_back(landmark1Texture);

    Texture landmark2Texture;
    landmark2Texture.id = loadTexture("assets/Landmark_2.png");
    if (landmark2Texture.id == 0) {
        std::cerr << "Failed to load Landmark2 texture!" << std::endl;
        return -1;
    }
    landmark2Texture.type = "texture_diffuse";
    landmark2Texture.path = "assets/Landmark_2.png";
    landmark2Textures.push_back(landmark2Texture);

    Mesh landmark1(landmark1Vertices, landmark1Indices, landmark1Textures);
    Mesh landmark2(landmark2Vertices, landmark2Indices, landmark2Textures);

    std::vector<Vertex> kartVertices;
    std::vector<unsigned int> kartIndices;
    if (!loadOBJ("assets/kart.obj", kartVertices, kartIndices)) {
        std::cerr << "Failed to load kart model!" << std::endl;
        return -1;
    }

    std::vector<Texture> kartTextures;
    Texture kartTexture;
    kartTexture.id = loadTexture("assets/kart.png");
    if (kartTexture.id == 0) {
        std::cerr << "Failed to load kart texture!" << std::endl;
        return -1;
    }
    kartTexture.type = "texture_diffuse";
    kartTexture.path = "assets/kart.png";
    kartTextures.push_back(kartTexture);

    std::vector<Texture> GhostkartTextures;
    Texture GhostkartTexture;
    GhostkartTexture.id = loadTexture("assets/ghostKart.png");
    if (GhostkartTexture.id == 0) {
        std::cerr << "Failed to load ghost kart texture!" << std::endl;
        return -1;
    }
    GhostkartTexture.type = "texture_diffuse";
    GhostkartTexture.path = "assets/ghostKart.png";
    GhostkartTextures.push_back(GhostkartTexture);

    std::vector<Texture> GhostkartTextures2;
    Texture GhostkartTexture2;
    GhostkartTexture2.id = loadTexture("assets/ghostKart2.png");
    if (GhostkartTexture2.id == 0) {
        std::cerr << "Failed to load ghost kart 2 texture!" << std::endl;
        return -1;
    }
    GhostkartTexture2.type = "texture_diffuse";
    GhostkartTexture2.path = "assets/ghostKart2.png";
    GhostkartTextures2.push_back(GhostkartTexture2);

    Mesh mainKart(kartVertices, kartIndices, kartTextures);
    Mesh ghostKart1(kartVertices, kartIndices, GhostkartTextures);
    Mesh ghostKart2(kartVertices, kartIndices, GhostkartTextures2);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    bool qPressed = false;
    bool ePressed = false;

    while (!glfwWindowShouldClose(window)) {
        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.ProcessKeyboard(window, deltaTime);

        const float BASE_TURN_RATE = 100.0f;
        const float REVERSE_TURN_MODIFIER = 0.7f;
        const float MIN_TURN_SPEED = 1.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            kartSpeed += ACCELERATION * deltaTime;
            if (kartSpeed > MAX_SPEED) kartSpeed = MAX_SPEED;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            kartSpeed -= ACCELERATION * deltaTime;
            if (kartSpeed < -MAX_SPEED / 2) kartSpeed = -MAX_SPEED / 2;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            float turnModifier = 1.0f;
            if (fabs(kartSpeed) < MIN_TURN_SPEED) {
                turnModifier = fabs(kartSpeed) / MIN_TURN_SPEED;
            }
            if (kartSpeed < 0) {
                turnModifier *= REVERSE_TURN_MODIFIER;
            }
            kartRotation += BASE_TURN_RATE * turnModifier * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            float turnModifier = 1.0f;
            if (fabs(kartSpeed) < MIN_TURN_SPEED) {
                turnModifier = fabs(kartSpeed) / MIN_TURN_SPEED;
            }
            if (kartSpeed < 0) {
                turnModifier *= REVERSE_TURN_MODIFIER;
            }
            kartRotation -= BASE_TURN_RATE * turnModifier * deltaTime;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE &&
            glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
            kartSpeed *= 0.65f;
            if (fabs(kartSpeed) < 0.1f) kartSpeed = 0.0f;
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zPressed) {
            cameraMode = (cameraMode == THIRD_PERSON) ? FIRST_PERSON : THIRD_PERSON;
            zPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) {
            zPressed = false;
        }

        kartPosition.x += kartSpeed * sin(glm::radians(kartRotation)) * deltaTime;
        kartPosition.z += kartSpeed * cos(glm::radians(kartRotation)) * deltaTime;

        if (!gameFinished) {
            
            if (!playerFinished && kartPosition.z >= FINISH_LINE_Z) {
                playerFinished = true;
                std::cout << "Player kart finished!" << std::endl;
            }

            if (!ghost1Finished && ghostKart1Position.z >= FINISH_LINE_Z) {
                ghost1Finished = true;
                std::cout << "Ghost kart 1 finished!" << std::endl;
            }

            if (!ghost2Finished && ghostKart2Position.z >= FINISH_LINE_Z) {
                ghost2Finished = true;
                std::cout << "Ghost kart 2 finished!" << std::endl;
            }

            if (playerFinished && ghost1Finished && ghost2Finished) {
                gameFinished = true;
                finishTime = glfwGetTime() - raceStartTime;

                std::cout << "\n=== RACE FINISHED ===" << std::endl;
                std::cout << "Total race time: " << finishTime << " seconds" << std::endl;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
            ghostKartsMoving = !ghostKartsMoving; 

            if (ghostKartsMoving) {
                
                raceStartTime = glfwGetTime();  
                gameFinished = false;
                playerFinished = false;
                ghost1Finished = false;
                ghost2Finished = false;

                ghostKart1Position = kartPosition;
                ghostKart2Position = kartPosition;

                ghostKart1Position.x += SIDE_KART_DISTANCE * cos(glm::radians(kartRotation));
                ghostKart1Position.z -= SIDE_KART_DISTANCE * sin(glm::radians(kartRotation));
                ghostKart2Position.x -= SIDE_KART_DISTANCE * cos(glm::radians(kartRotation));
                ghostKart2Position.z += SIDE_KART_DISTANCE * sin(glm::radians(kartRotation));

                sideKartRotation = kartRotation;
            }

            spacePressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
            spacePressed = false;
        }

        if (ghostKartsMoving) {
            
            ghostKart1Position.x += SIDE_KART_MAX_SPEED * sin(glm::radians(sideKartRotation)) * deltaTime;
            ghostKart1Position.z += SIDE_KART_MAX_SPEED * cos(glm::radians(sideKartRotation)) * deltaTime;

            ghostKart2Position.x += SIDE_KART2_MAX_SPEED * sin(glm::radians(sideKartRotation)) * deltaTime;
            ghostKart2Position.z += SIDE_KART2_MAX_SPEED * cos(glm::radians(sideKartRotation)) * deltaTime;
        }

        camera.FollowKart(kartPosition, kartRotation, cameraMode);

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !qPressed) {
            currentSkybox = DAY;
            directionalLight.update(true);
            qPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) {
            currentSkybox = NIGHT;
            directionalLight.update(false);
            ePressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) qPressed = false;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) ePressed = false;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

        directionalLight.update(currentSkybox == DAY);
        directionalLight.applyToShader(groundShader, "dirLight");

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setInt("skybox", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, currentSkybox == DAY ? dayCubemap : nightCubemap);

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        groundShader.use();
        groundShader.setVec3("viewPos", camera.Position);
        glm::mat4 model = glm::mat4(1.0f);
        groundShader.setMat4("model", model);
        groundShader.setMat4("view", view);
        groundShader.setMat4("projection", projection);
        groundShader.setInt("texture1", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (!gameFinished) {
            groundShader.use();

            glm::mat4 finishLineModel = glm::mat4(1.0f);
            finishLineModel = glm::translate(finishLineModel, glm::vec3(0.0f, 0.01f, FINISH_LINE_Z));

            finishLineModel = glm::scale(finishLineModel, glm::vec3(1.0f, 0.001f, 0.1f));

            groundShader.setMat4("model", finishLineModel);
            groundShader.setMat4("view", view);
            groundShader.setMat4("projection", projection);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, finishLineTexture);
            groundShader.setInt("texture1", 0);

            glBindVertexArray(planeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        kartShader.use();

        kartShader.setVec3("viewPos", camera.Position);
        kartShader.setFloat("material.shininess", 32.0f);
        directionalLight.applyToShader(kartShader, "dirLight");

        glm::mat4 landmark1Model = glm::mat4(1.0f);
        landmark1Model = glm::translate(landmark1Model,
            glm::vec3(-LANDMARK_SPACING / 2, 0.0f, FINISH_LINE_Z + LANDMARK_DISTANCE_FROM_FINISH));
        float rotationAngle = glm::radians(495.0f); 
        landmark1Model = glm::rotate(landmark1Model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); 
        landmark1Model = glm::scale(landmark1Model, glm::vec3(0.0099f)); 
        kartShader.setMat4("model", landmark1Model);
        landmark1.Draw(kartShader);

        glm::mat4 landmark2Model = glm::mat4(1.0f);
        landmark2Model = glm::translate(landmark2Model,
            glm::vec3(LANDMARK_SPACING / 2, 0.0f, FINISH_LINE_Z + LANDMARK_DISTANCE_FROM_FINISH));
        rotationAngle = glm::radians(45.0f); 
        landmark2Model = glm::rotate(landmark2Model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        landmark2Model = glm::scale(landmark2Model, glm::vec3(0.0099f)); 
        kartShader.setMat4("model", landmark2Model);
        landmark2.Draw(kartShader);

        glm::mat4 kartModel = glm::mat4(1.0f);
        kartModel = glm::translate(kartModel, kartPosition);
        kartModel = glm::rotate(kartModel, glm::radians(kartRotation + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        kartModel = glm::scale(kartModel, glm::vec3(0.009f));
        kartShader.setMat4("model", kartModel);
        kartShader.setMat4("view", view);
        kartShader.setMat4("projection", projection);
        mainKart.Draw(kartShader, 1.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); 

        
        std::vector<std::pair<float, glm::vec3>> transparentObjects;
        transparentObjects.push_back({ glm::distance(camera.Position, ghostKart1Position), ghostKart1Position });
        transparentObjects.push_back({ glm::distance(camera.Position, ghostKart2Position), ghostKart2Position });

        std::sort(transparentObjects.begin(), transparentObjects.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

       
        for (const auto& obj : transparentObjects) {
            glm::mat4 ghostModel = glm::mat4(1.0f);
            ghostModel = glm::translate(ghostModel, obj.second);
            ghostModel = glm::rotate(ghostModel, glm::radians(kartRotation + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ghostModel = glm::scale(ghostModel, glm::vec3(0.009f));
            kartShader.setMat4("model", ghostModel);

            if (obj.second == ghostKart1Position) {
                ghostKart1.Draw(kartShader, 0.5f); 
            }
            else {
                ghostKart2.Draw(kartShader, 0.5f); 
            }
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteTextures(1, &dayCubemap);
    glDeleteTextures(1, &nightCubemap);
    glDeleteTextures(1, &groundTexture);
    glDeleteTextures(1, &kartTexture.id);
    glfwTerminate();
    return 0;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "Failed to load cubemap texture: " << faces[i] << std::endl;
            stbi_image_free(data);
            return 0;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture at " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = 400, lastY = 300;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return; 
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void checkTextureLoading(const std::vector<std::string>& faces) {
    for (const auto& path : faces) {
        std::ifstream file(path);
        if (!file.good()) {
            std::cerr << "Texture file not found: " << path << std::endl;
        }
        else {
            std::cout << "Texture found: " << path << std::endl;
        }
    }
}

bool loadOBJ(const std::string& path, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<unsigned int> positionIndices, normalIndices, texCoordIndices;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            
            std::istringstream ss(line.substr(2));
            glm::vec3 position;
            ss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (line.substr(0, 3) == "vn ") {
            
            std::istringstream ss(line.substr(3));
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (line.substr(0, 3) == "vt ") {
           
            std::istringstream ss(line.substr(3));
            glm::vec2 texCoord;
            ss >> texCoord.x >> texCoord.y;
            texCoord.y = 1.0f - texCoord.y; 
            temp_texCoords.push_back(texCoord);
        }
        else if (line.substr(0, 2) == "f ") {
           
            std::istringstream ss(line.substr(2));
            std::string vertexData;
            while (ss >> vertexData) {
                std::vector<std::string> data = split(vertexData, '/');

                unsigned int posIndex = std::stoi(data[0]) - 1;
                unsigned int texIndex = data[1].empty() ? 0 : std::stoi(data[1]) - 1;
                unsigned int normIndex = std::stoi(data[2]) - 1;

                positionIndices.push_back(posIndex);
                texCoordIndices.push_back(texIndex);
                normalIndices.push_back(normIndex);
            }
        }
    }

    for (unsigned int i = 0; i < positionIndices.size(); i++) {
        Vertex vertex;
        vertex.position = temp_positions[positionIndices[i]];
        vertex.normal = temp_normals[normalIndices[i]];
        vertex.texCoords = temp_texCoords[texCoordIndices[i]];
        vertices.push_back(vertex);
        indices.push_back(i);
    }

    return true;
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}