#include <fstream>      // For file input (reading shader files)
#include <sstream>      // For string stream operations (combining file content)
#include <iostream>     // For console output (useful for debugging)
#include <glm/glm.hpp>  // GLM library for vector and matrix types (used in uniforms)

// A Shader class to handle compiling and using vertex/fragment shaders
class Shader {
public:
    GLuint ID; // Shader program ID that OpenGL uses to reference the compiled program

    // Constructor: loads, compiles, and links the vertex and fragment shaders
    Shader(const char* vertexPath, const char* fragmentPath) {
        // Read vertex shader source code from file
        std::fstream vertSrc(vertexPath);
        std::stringstream vertBuff;
        vertBuff << vertSrc.rdbuf();         // Load entire file into buffer
        std::string vertS = vertBuff.str();  // Convert buffer to string
        const char* v = vertS.c_str();       // Get C-style string for OpenGL

        // Read fragment shader source code from file
        std::fstream fragSrc(fragmentPath);
        std::stringstream fragBuff;
        fragBuff << fragSrc.rdbuf();         // Load entire file into buffer
        std::string fragS = fragBuff.str();  // Convert buffer to string
        const char* f = fragS.c_str();       // Get C-style string for OpenGL

        GLuint vertex, fragment;

        // Compile vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);      // Create shader object
        glShaderSource(vertex, 1, &v, NULL);             // Attach shader source code
        glCompileShader(vertex);                         // Compile the shader

        // Compile fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);   // Create shader object
        glShaderSource(fragment, 1, &f, NULL);           // Attach shader source code
        glCompileShader(fragment);                       // Compile the shader

        // Link shaders into a shader program
        ID = glCreateProgram();                          // Create shader program object
        glAttachShader(ID, fragment);                    // Attach fragment shader
        glAttachShader(ID, vertex);                      // Attach vertex shader
        glLinkProgram(ID);                               // Link both shaders into the program

        // Delete individual shaders after linking (no longer needed separately)
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Activate the shader program
    void use() {
        glUseProgram(ID);
    }

    // Utility functions to set uniform variables in the shader
    // (Uniforms are used to send data from CPU to GPU)

    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setMat2(const std::string& name, const glm::mat2& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string& name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

};
