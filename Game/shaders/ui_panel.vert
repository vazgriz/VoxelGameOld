#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 positions[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(push_constant) uniform Panel {
    vec4 color;
	vec4 pos;
    vec2 size;
} panel;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 position = vec4(positions[gl_VertexIndex] * panel.size, 1.0, 1.0) + panel.pos;
    gl_Position = ubo.proj * position;
    outColor = panel.color;
}