#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler mainSampler;
layout (set = 0, binding = 1) uniform texture2D mainTexture;

void main() {
    outColor = texture(sampler2D(mainTexture, mainSampler), inUv);
}
