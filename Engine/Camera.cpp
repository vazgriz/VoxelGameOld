#include "Engine/Camera.h"
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace VoxelEngine;

Camera::Camera(uint32_t width, uint32_t height, float fov, float nearPlane, float farPlane) {
    setFOV(fov);
    setNearPlane(nearPlane);
    setFarPlane(farPlane);
}

void Camera::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    createProjectionMatrix();
}

void Camera::setFOV(float value) {
    m_fov = value;
    createProjectionMatrix();
}

void Camera::setNearPlane(float value) {
    m_nearPlane = value;
    createProjectionMatrix();
}

void Camera::setFarPlane(float value) {
    m_farPlane = value;
    createProjectionMatrix();
}

void Camera::createProjectionMatrix() {
    m_projectionMatrix = glm::perspectiveFovRH_ZO(m_fov, static_cast<float>(m_width), static_cast<float>(m_height), m_nearPlane, m_farPlane);
    m_projectionMatrix[1][1] *= -1; //flip Y coordinate
}

void Camera::setPosition(glm::vec3 position) {
    m_position = position;
    createViewMatrix();
}

void Camera::setRotation(glm::quat rotation) {
    m_rotation = rotation;
    createViewMatrix();
}

void Camera::createViewMatrix() {
    m_viewMatrix = glm::lookAt(m_rotation * glm::vec3(1, 0, 0), m_rotation * glm::vec3(0, 1, 0), m_rotation * glm::vec3(0, 0, 1)) * glm::translate(m_position);
}