#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <unordered_map>

class ShaderProgram {
   public:
    ShaderProgram(const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& out_color_name);
    ShaderProgram(const std::string& vertex_shader_file, const std::string& fragment_shader_file) : ShaderProgram(vertex_shader_file, fragment_shader_file, "out_color"){};
    ShaderProgram(const std::string& shader_name) : ShaderProgram(shader_name + ".vert", shader_name + ".frag"){};
    GLuint gl_program_id;
    GLuint vao;

    void use();

    GLuint point_attribute(const std::string& attribute_name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data);
    GLint get_uniform_location(const std::string& uniform_name);

    void set_uniform(const std::string& uniform_name, float f);
    void set_uniform(const std::string& uniform_name, glm::vec2 v);
    void set_uniform(const std::string& uniform_name, glm::vec3 v);
    void set_uniform(const std::string& uniform_name, glm::vec4 v);
    void set_uniform(const std::string& uniform_name, glm::mat2 m);
    void set_uniform(const std::string& uniform_name, glm::mat3 m);
    void set_uniform(const std::string& uniform_name, glm::mat4 m);
    void set_uniform(const std::string& uniform_name, glm::uvec3 v);

   private:
    std::unordered_map<std::string, GLint> uniform_locations;
};