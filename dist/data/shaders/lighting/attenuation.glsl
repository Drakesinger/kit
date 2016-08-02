#version 410 core

float calcAttenuation(vec3 in_lightPos, vec3 in_surfacePos, vec4 in_lightFalloff)
{
    vec3 objToLightVec = in_lightPos - in_surfacePos;
    float len_sq = dot(objToLightVec, objToLightVec);
    float len = sqrt(len_sq);
    float attenuation = dot(in_lightFalloff.yzw, vec3(1.0, len, len_sq));
    attenuation = 1.0 / attenuation;
    float dist = distance(in_lightPos, in_surfacePos);
    float att = clamp(1.0 - dist*dist/((in_lightFalloff.x*2)*(in_lightFalloff.x*2)), 0.0, 1.0);
    att =  att * att;
    return attenuation * att;
}