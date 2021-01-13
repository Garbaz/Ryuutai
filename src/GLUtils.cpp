#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "GLUtils.hpp"

GLFWwindow *init_glfw(const std::string& window_name) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
    // window = glfwCreateWindow(vidmode->width, vidmode->height, "RaytraceGL", monitor, NULL);
    GLFWwindow *window = glfwCreateWindow(1200, 675, window_name.c_str(), NULL, NULL);

    glfwMakeContextCurrent(window);

    return window;
}

void GLAPIENTRY message_callback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar *message,
                                 const void *user_param) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

void init_glew() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "glew init error: " << glewGetErrorString(err) << std::endl;
        exit(-1);
    }

#if PRINT_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
#endif
}

std::string read_file(std::string filename) {
    std::ifstream f(filename);
    if (f) {
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    } else {
        throw "Could not read file \"" + filename + "\"";
    }
}

GLuint compile_shader(std::string filename, GLenum shader_type) {
    std::string source;
    try {
        source = read_file(filename);
    } catch (std::string err) {
        std::cerr << "ERROR: " << err << std::endl;
        exit(-1);
    }
    const char *source_cstr = source.c_str();

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source_cstr, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
#if PRINT_DEBUG
    std::cerr << "Shader \"" << filename << "\" compiled " << (status == GL_TRUE ? "successful" : "unsuccessful") << "." << std::endl;

    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        char info_log[info_log_length];
        glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
        std::cerr << "Shader compolation log:" << std::endl;
        std::cerr << info_log << std::endl;
    }
#endif
    if (status != GL_TRUE) {
        exit(-1);
    }
    return shader;
}

GLuint compiler_render_program(std::string vertex_shader_filename, std::string fragment_shader_filename, std::string out_color_name) {
    GLuint vertex_shader = compile_shader(vertex_shader_filename, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_filename, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindFragDataLocation(program, 0, out_color_name.c_str());
    glLinkProgram(program);

    return program;
}



glm::vec3 random_saturated_color() {
    float m = glm::linearRand(0, 1);
    switch (rand() % 6) {
        case 0:
            return glm::vec3(1, 0, m);
        case 1:
            return glm::vec3(1, m, 0);
        case 2:
            return glm::vec3(0, m, 1);
        case 3:
            return glm::vec3(0, 1, m);
        case 4:
            return glm::vec3(m, 1, 0);
        case 5:
            return glm::vec3(m, 0, 1);
    }
}
