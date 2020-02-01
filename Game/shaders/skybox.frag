#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler textureSampler;
layout (set = 1, binding = 1) uniform textureCube textureArray;

void main() {
    outColor = vec4(texture(samplerCube(textureArray, textureSampler), fragPosition).xyz, 1.0);
    //outColor = vec4(normalize(fragPosition), 1.0);
}
