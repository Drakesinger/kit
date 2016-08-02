#version 410 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec3 out_color;

uniform sampler2D uniform_sourceTexture;

uniform float uniform_whitepoint = 1.0;
uniform float uniform_exposure = 1.0;
uniform float uniform_tresholdBias = 0.0;

vec3 lumaweights = vec3(0.2125f,0.7154f,0.0721f); // Reinhard

void main( ) {
    vec3 color = texture(uniform_sourceTexture, in_uv).rgb;
    float luma = dot(color * uniform_exposure, lumaweights);
    luma = max(luma - uniform_whitepoint + uniform_tresholdBias, 0.0);
    out_color = color * sign(luma);
}