#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D normal;
uniform sampler2D worldPos;
uniform sampler2D ssaoNoise;

layout (std140) uniform Globals {
    mat4 projView;
    mat4 proj;
    mat4 view;
    uvec2 viewportSize;
    vec4 ssaoKernels[64];
    float globalTime;
    float dayTime;
    float gamma;
};

const vec2 noiseScale = vec2(4.0f, 4.0f);
const int kernelSize = 64;
const float radius = 0.5f;
const float bias = 0.025f;

void main() {
    vec3 normal = texture(normal, uv).xyz;
    vec3 fragPos = texture(worldPos, uv).xyz;
    vec2 noiseUv = uv * viewportSize / noiseScale;

    vec3 randomVec = texture(ssaoNoise, noiseUv).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);  

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; i++)
    {
        vec3 samplePos = TBN * ssaoKernels[i].xyz;
        samplePos = fragPos + samplePos * radius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset = proj * offset;
        offset.xyz /= offset.w;
        offset.xyz  = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(worldPos, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0f - (occlusion / kernelSize);

    fragColor = vec4(vec3(occlusion), 1.0f);
}
