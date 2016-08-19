#version 410 core

float calcSpotAttenuation(vec3 in_lightPos, vec3 in_surfacePos, vec2 in_lightConeAngle, vec3 in_lightDirection)
{
    vec3 objToLightVec = in_lightPos - in_surfacePos; 
    float len_sq = dot(objToLightVec, objToLightVec); 
    float len = sqrt(len_sq); 
    vec3 objToLightDir = objToLightVec / len; 
    float spotlightAngle = clamp((dot(in_lightDirection, -objToLightDir)), 0.0, 1.0);
    float spotFalloff = clamp((spotlightAngle - in_lightConeAngle.x) / (in_lightConeAngle.y - in_lightConeAngle.x), 0.0, 1.0);
    return (1.0 - spotFalloff);
}