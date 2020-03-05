#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler textureSampler;
layout (set = 1, binding = 1) uniform texture2D textureImage;

void main() {
    outColor = texture(sampler2D(textureImage, textureSampler), fragUV);
}
