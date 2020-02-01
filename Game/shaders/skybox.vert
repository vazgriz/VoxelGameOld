#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragPosition;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);

void main() {
    mat4 view = mat4(mat3(
        ubo.view[0].xyz, 
        ubo.view[1].xyz, 
        ubo.view[2].xyz
    ));

    vec4 position = vec4(positions[gl_VertexIndex], 1.0, 1.0);
    gl_Position = position;
    fragPosition = (inverse(ubo.proj * view) * position).xyz;
}