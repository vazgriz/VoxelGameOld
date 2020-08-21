#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);

layout(push_constant) uniform Transform {
	ivec4 pos;
} transform;

void main() {
    vec4 position = vec4(positions[gl_VertexIndex], 1.0, 1.0);
    gl_Position = position;
}