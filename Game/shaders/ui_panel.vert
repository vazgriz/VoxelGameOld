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

layout(push_constant) uniform Transform {
    vec4 color;
	vec4 pos;
    vec2 size;
} transform;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 position = vec4(positions[gl_VertexIndex] * transform.size, 1.0, 1.0) + transform.pos;
    gl_Position = position;
    outColor = transform.color;
}