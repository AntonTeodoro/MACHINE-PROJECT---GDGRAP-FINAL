#pragma once
#include <glm/glm.hpp> // Include GLM for vector math (vec3)

// Class representing a directional light source (like the sun)
class Light {
public:

    // Light properties using GLM vectors
    glm::vec3 direction;  // Direction the light is pointing (for directional lighting)
    glm::vec3 ambient;    // Ambient light color (general background light)
    glm::vec3 diffuse;    // Diffuse light color (main light based on angle)
    glm::vec3 specular;   // Specular light color (highlights on shiny surfaces)
    float intensity;      // Multiplier for overall light strength

    // Constructor to initialize the light with values
    Light(glm::vec3 dir, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float intense)
        : direction(dir), ambient(amb), diffuse(diff), specular(spec), intensity(intense) {
    }

    // Update the light settings depending on whether it's day or night
    void update(bool isDay) {
        if (isDay) {
            // Brighter, warmer lighting for day
            direction = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f)); // Direction pointing diagonally down
            ambient = glm::vec3(0.3f, 0.3f, 0.3f);                      // Soft general light
            diffuse = glm::vec3(0.8f, 0.8f, 0.7f);                      // Bright but slightly yellowish sunlight
            specular = glm::vec3(0.5f, 0.5f, 0.5f);                     // Shiny highlights
            intensity = 1.0f;                                           // Full strength
        }
        else {
            // Dimmer, cooler lighting for night
            direction = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)); // Direction still down, but different angle
            ambient = glm::vec3(0.1f, 0.1f, 0.15f);                     // Very soft ambient light, bluish
            diffuse = glm::vec3(0.3f, 0.3f, 0.4f);                      // Dim moonlight effect
            specular = glm::vec3(0.1f, 0.1f, 0.1f);                     // Subtle highlights
            intensity = 0.5f;                                           // Reduced strength
        }
    }

    // Sends light data to the shader using a uniform variable prefix
    // This assumes the shader has uniform variables like "light.direction", etc.
    void applyToShader(Shader& shader, const std::string& uniformPrefix) {
        shader.setVec3(uniformPrefix + ".direction", direction);
        shader.setVec3(uniformPrefix + ".ambient", ambient * intensity);
        shader.setVec3(uniformPrefix + ".diffuse", diffuse * intensity);
        shader.setVec3(uniformPrefix + ".specular", specular * intensity);
    }
};
