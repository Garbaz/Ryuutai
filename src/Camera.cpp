#include <glm/ext.hpp>

#include "Camera.hpp"

Camera::Camera(glm::vec3 _position, double _angle_horizontal, double _angle_vertical) {
    position = _position;
    angle_horizontal = _angle_horizontal;
    angle_vertical = _angle_vertical;
    update_direction(0, 0);
}

void Camera::update(float dt) {
    if (input_movement.x != 0 || input_movement.y != 0) {
        glm::vec3 dir_right = glm::vec3(direction.z, 0, -direction.x);
        //glm::vec3 dir_forward = glm::vec3(direction.x, 0, direction.z);
        glm::vec3 dir_forward = direction;
        glm::vec3 move = FLY_SPEED * (float(input_movement.x) * dir_right + float(input_movement.y) * dir_forward);
        update_position(position + move);
    }
}

void Camera::update_position(glm::vec3 new_position) {
    position = new_position;
    dirty = true;
}

void Camera::update_direction(glm::vec3 new_direction) {
    direction = new_direction;
    dirty = true;
}

void Camera::update_direction(double delta_pos_x, double delta_pos_y) {
    angle_horizontal -= CAMERA_MOUSE_SENS * delta_pos_x;
    angle_vertical -= CAMERA_MOUSE_SENS * delta_pos_y;
    if (angle_vertical > 0.99 * glm::half_pi<double>()) {
        angle_vertical = 0.99 * glm::half_pi<double>();
    } else if (angle_vertical < -0.99 * glm::half_pi<double>()) {
        angle_vertical = -0.99 * glm::half_pi<double>();
    }
    update_direction(glm::vec3(glm::cos(angle_vertical) * glm::sin(angle_horizontal),
                               glm::sin(angle_vertical),
                               glm::cos(angle_vertical) * glm::cos(angle_horizontal)));
}

const glm::mat4& Camera::get_view_matrix() {
    if (dirty) {
        view_matrix = glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
        dirty = false;
    }
    return view_matrix;
}

const glm::vec3& Camera::get_position() {
    return position;
}
const glm::vec3& Camera::get_direction() {
    return direction;
}