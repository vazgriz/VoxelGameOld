#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragUV;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler textureSampler;
layout (set = 1, binding = 1) uniform texture2DArray textureArray;

void main() {
    outColor = vec4(fragColor * texture(sampler2DArray(textureArray, textureSampler), fragUV).xyz, 1.0);
}
