#pragma once

#include <glm/glm.hpp>

const float FIELD_OF_VIEW_HORIZONTAL = 100.0;
const float CAMERA_MOUSE_SENS = 5.0 * glm::radians(0.022);
const float FLY_SPEED = 0.1;

class Camera {
   public:
    Camera(glm::vec3 _position, double _angle_horizontal, double _angle_vertical);
    Camera(glm::vec3 _position) : Camera(_position,0,0) {};
    Camera() : Camera(glm::vec3(0)) {};
    bool dirty;
    glm::ivec2 input_movement;
    void update(float dt);
    void update_position(glm::vec3 new_position);
    void update_direction(double delta_pos_x, double delta_pos_y);
    const glm::mat4& get_view_matrix();
    const glm::vec3& get_position();
    const glm::vec3& get_direction();

   private:
    void update_direction(glm::vec3 new_direction);
    glm::mat4 view_matrix;
    glm::vec3 position;
    glm::vec3 direction;
    double angle_horizontal;
    double angle_vertical;
};