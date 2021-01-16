#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <string>

#include "GLUtils.hpp"

#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& out_color_name) {
    gl_program_id = compiler_render_program(vertex_shader_file, fragment_shader_file, out_color_name);
    glGenVertexArrays(1, &vao);
}

void ShaderProgram::use() {
    glBindVertexArray(vao);
    glUseProgram(gl_program_id);
}

GLuint ShaderProgram::point_attribute(const std::string& attribute_name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data) {
    glBindVertexArray(vao);
    glUseProgram(gl_program_id);
    GLint loc = glGetAttribLocation(gl_program_id, attribute_name.c_str());
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, size, type, normalized, stride, data);
    glBindVertexArray(0);
    glUseProgram(0);
}

GLint ShaderProgram::get_uniform_location(const std::string& uniform_name) {
    auto find = uniform_locations.find(uniform_name);
    GLint loc;
    if (find == uniform_locations.end()) {
        loc = glGetUniformLocation(gl_program_id, uniform_name.c_str());
        uniform_locations[uniform_name] = loc;
    } else {
        loc = find->second;
    }
    return loc;
}

void ShaderProgram::set_uniformf(const std::string& uniform_name, float f) {
    glUseProgram(gl_program_id);
    glUniform1f(get_uniform_location(uniform_name), f);
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::vec2 v) {
    glUseProgram(gl_program_id);
    glUniform2fv(get_uniform_location(uniform_name), 1, glm::value_ptr(v));
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::vec3 v) {
    glUseProgram(gl_program_id);
    glUniform3fv(get_uniform_location(uniform_name), 1, glm::value_ptr(v));
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::vec4 v) {
    glUseProgram(gl_program_id);
    glUniform4fv(get_uniform_location(uniform_name), 1, glm::value_ptr(v));
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::mat2 m) {
    glUseProgram(gl_program_id);
    glUniformMatrix2fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(m));
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::mat3 m) {
    glUseProgram(gl_program_id);
    glUniformMatrix3fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(m));
    glUseProgram(0);
}
void ShaderProgram::set_uniformf(const std::string& uniform_name, glm::mat4 m) {
    glUseProgram(gl_program_id);
    glUniformMatrix4fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(m));
    glUseProgram(0);
}
void ShaderProgram::set_uniformu(const std::string& uniform_name, glm::uvec3 v) {
    glUseProgram(gl_program_id);
    glUniform3uiv(get_uniform_location(uniform_name), 1, glm::value_ptr(v));
    glUseProgram(0);
}
void ShaderProgram::set_uniformi(const std::string& uniform_name, int n) {
    glUseProgram(gl_program_id);
    glUniform1i(get_uniform_location(uniform_name), n);
    glUseProgram(0);
}