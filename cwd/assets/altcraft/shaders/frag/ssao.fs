#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D normal;
uniform sampler2D light;
uniform sampler2D depthStencil;
uniform sampler2D ssaoNoise;

uniform int ssaoSamples;

layout (std140) uniform Globals {
    mat4 projView;
    mat4 proj;
    mat4 invProj;
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

vec3 RecoverViewWorldPos(vec2 screenPos, float depth) {
    vec4 viewPos = invProj * vec4(screenPos * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return viewPos.xyz / viewPos.w;
}

void main() {
    vec3 normal = texture(normal, uv).xyz;
    vec3 fragPos = RecoverViewWorldPos(uv, texture(depthStencil, uv).r);
    vec2 noiseUv = uv * viewportSize / noiseScale;

    vec3 randomVec = texture(ssaoNoise, noiseUv).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);  

    float occlusion = 0.0;
    int samples = min(kernelSize, ssaoSamples);
    for(int i = 0; i < samples; i++)
    {
        vec3 samplePos = TBN * ssaoKernels[i].xyz;
        samplePos = fragPos + samplePos * radius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset = proj * offset;
        offset.xyz /= offset.w;
        offset.xyz  = offset.xyz * 0.5 + 0.5;

        float sampleDepth = RecoverViewWorldPos(offset.xy, texture(depthStencil, offset.xy).r).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        float aoMask = texture(light, offset.xy).b;
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck * aoMask;
    }

    occlusion = 1.0f - (occlusion / samples);

    fragColor = vec4(vec3(occlusion), 1.0f);
}
