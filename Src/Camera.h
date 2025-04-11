#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Enum to define camera modes
enum CameraMode {
    THIRD_PERSON,
    FIRST_PERSON
};

// Camera class to handle movement, view matrix, and perspective
class Camera {
public:
    // Default mode is third-person
    CameraMode currentMode = THIRD_PERSON;

    // Constructor with optional parameters for initial position, up vector, yaw, and pitch
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(2.5f),
        MouseSensitivity(0.05f),
        Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors(); // Calculate initial Front, Right, Up vectors
    }

    // Returns the view matrix calculated using the camera's position and direction
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Handles keyboard input for movement
    void ProcessKeyboard(GLFWwindow* window, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            Position += Front * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            Position -= Front * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            Position -= Right * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            Position += Right * velocity;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            Position += WorldUp * velocity;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            Position -= WorldUp * velocity;
    }

    // Handles mouse movement for looking around (only in THIRD_PERSON mode)
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        if (currentMode != THIRD_PERSON) {
            std::cout << "Mouse input ignored (not in THIRD_PERSON mode)" << std::endl;
            return;
        }

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Clamp pitch to avoid flipping
        if (constrainPitch) {
            Pitch = glm::clamp(Pitch, -89.0f, 89.0f);
        }

        updateCameraVectors();
    }

    // Handles mouse scroll for zooming
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    // Public camera properties
    glm::vec3 Position; // Camera position
    glm::vec3 Front;    // Direction the camera is facing
    glm::vec3 Up;       // Up vector relative to camera
    glm::vec3 Right;    // Right vector relative to camera
    glm::vec3 WorldUp;  // Global up direction (usually Y-axis)

    float Yaw;          // Horizontal rotation
    float Pitch;        // Vertical rotation

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Toggle between third- and first-person views
    void ToggleCameraMode() {
        currentMode = (currentMode == THIRD_PERSON) ? FIRST_PERSON : THIRD_PERSON;
    }

    // Updates camera position and orientation to follow a "kart" (player/vehicle)
    void FollowKart(const glm::vec3& kartPos, float kartRotation, CameraMode mode) {
        currentMode = mode;

        if (mode == THIRD_PERSON) {
            // Third-person view: camera is behind and above the kart
            float distance = 7.0f;
            float height = 2.0f;

            float offsetX = distance * sin(glm::radians(kartRotation + Yaw + 180.0f));
            float offsetZ = distance * cos(glm::radians(kartRotation + Yaw + 180.0f));

            Position = kartPos + glm::vec3(offsetX, height, offsetZ);

            // Camera looks slightly ahead of the kart
            glm::vec3 lookAt = kartPos + glm::vec3(
                sin(glm::radians(kartRotation + Yaw)) * 3.0f,
                0.5f,
                cos(glm::radians(kartRotation + Yaw)) * 3.0f
            );

            Front = glm::normalize(lookAt - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));
        }
        else {
            // First-person view: camera is in front of the kart
            float forwardOffset = 1.5f;
            float forwardX = sin(glm::radians(kartRotation)) * forwardOffset;
            float forwardZ = cos(glm::radians(kartRotation)) * forwardOffset;

            Position = kartPos + glm::vec3(forwardX, 0.5f, forwardZ);
            Front = glm::vec3(
                sin(glm::radians(kartRotation)),
                0.0f,
                cos(glm::radians(kartRotation))
            );
        }
    }

private:
    // Updates camera's Front, Right, and Up vectors based on Yaw and Pitch
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp)); // Right vector
        Up = glm::normalize(glm::cross(Right, Front));      // Recalculate Up vector
    }
};
