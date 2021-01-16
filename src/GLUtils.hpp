#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <string>


GLFWwindow *init_glfw(const std::string& window_name);
void init_glew();
GLuint compile_shader(std::string filename, GLenum shader_type);
GLuint compiler_render_program(std::string vertex_shader_filename, std::string fragment_shader_filename, std::string out_color_name);
glm::vec3 random_saturated_color(float m, int i);
glm::vec3 random_saturated_color();

