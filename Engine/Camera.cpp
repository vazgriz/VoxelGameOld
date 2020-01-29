#include "Engine/Camera.h"
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace VoxelEngine;

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
    glm::vec3 v = p1 - p0;
    glm::vec3 u = p2 - p0;
    glm::vec3 n = glm::normalize(glm::cross(v, u));

    components.x = n.x;
    components.y = n.y;
    components.z = n.z;
    components.w = -glm::dot(n, p0);
}

Plane::Plane(glm::vec4 plane) {
    components = plane;
}

float Plane::distance(glm::vec3 p) {
    return glm::dot(glm::vec3(components.x, components.y, components.z), p) + components.w;
}

bool Plane::testAABB(glm::vec3 origin, glm::vec3 extent) {
    glm::vec3 min = origin;

    if (components.x >= 0) {
        min.x += extent.x;
    }

    if (components.y >= 0) {
        min.y += extent.y;
    }

    if (components.z >= 0) {
        min.z += extent.z;
    }

    if (distance(min) < 0) return false;

    return true;
}

bool Frustum::testAABB(glm::vec3 origin, glm::vec3 extent) {
    return near.testAABB(origin, extent)
        && far.testAABB(origin, extent)
        && left.testAABB(origin, extent)
        && right.testAABB(origin, extent)
        && top.testAABB(origin, extent)
        && bottom.testAABB(origin, extent);
}

Camera::Camera(Engine& engine, uint32_t width, uint32_t height, float fov, float nearPlane, float farPlane) {
    m_engine = &engine;
    m_width = width;
    m_height = height;

    m_position = {};
    m_rotation = glm::identity<glm::quat>();

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
    glm::vec3 right = m_rotation * glm::vec3(1, 0, 0);
    glm::vec3 up = m_rotation * glm::vec3(0, 1, 0);
    glm::vec3 forward = m_rotation * glm::vec3(0, 0, -1);
    m_viewMatrix = glm::lookAtRH(m_position, m_position + forward, up);

    createFrustum(forward, up, right);
}

void Camera::createFrustum(glm::vec3 forward, glm::vec3 up, glm::vec3 right) {
    float aspect = static_cast<float>(m_width) / m_height;

    float nearHeight = 2 * glm::tan(m_fov / 2) * m_nearPlane;
    float nearWidth = nearHeight * aspect;

    float farHeight = 2 * glm::tan(m_fov / 2) * m_farPlane;
    float farWidth = farHeight * aspect;

    glm::vec3 farCenter = m_position + m_farPlane * forward;
    glm::vec3 nearCenter = m_position + m_nearPlane * forward;

    glm::vec3 farBottomLeft = farCenter - (up * (farHeight / 2)) - (right * (farWidth / 2));
    glm::vec3 farBottomRight = farBottomLeft + right * farWidth;
    glm::vec3 farTopLeft = farBottomLeft + up * farHeight;
    glm::vec3 farTopRight = farBottomRight + up * farHeight;

    glm::vec3 nearBottomLeft = nearCenter - (up * (nearHeight / 2)) - (right * (nearWidth / 2));
    glm::vec3 nearBottomRight = nearBottomLeft + right * nearWidth;
    glm::vec3 nearTopLeft = nearBottomLeft + up * nearHeight;
    glm::vec3 nearTopRight = nearBottomRight + up * nearHeight;

    m_frustum.near = Plane(nearBottomRight, nearBottomLeft, nearTopRight);
    m_frustum.far = Plane(farBottomLeft, farBottomRight, farTopLeft);
    m_frustum.left = Plane(nearBottomLeft, farBottomLeft, nearTopLeft);
    m_frustum.right = Plane(farBottomRight, nearBottomRight, farTopRight);
    m_frustum.top = Plane(nearTopRight, nearTopLeft, farTopRight);
    m_frustum.bottom = Plane(nearBottomLeft, nearBottomRight, farBottomLeft);
}