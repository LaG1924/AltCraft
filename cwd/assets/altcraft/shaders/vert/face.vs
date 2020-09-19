#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec4 Texture;
layout (location = 8) in mat4 model;
layout (location = 12) in vec3 color;
layout (location = 13) in vec2 light;
layout (location = 14) in float TextureLayer;
layout (location = 15) in float TextureFrames;

out VS_OUT {
    vec2 UvPosition;
    vec3 Texture;
    vec3 Color;
    vec2 Light;
} vs_out;

uniform float GlobalTime;
uniform mat4 projView;

vec3 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords, float Layer) {
    float x = TextureAtlasCoords.x;
    float y = TextureAtlasCoords.y;
//     float w = TextureAtlasCoords.z;
    float h = TextureAtlasCoords.w;
	vec2 transformed = vec2(x, 1 - y - h) + UvCoords * TextureAtlasCoords.zw;
    return vec3(transformed.x, transformed.y, Layer);
}

void main()
{
    vec4 sourcePosition = vec4(position,1.0f);
    gl_Position = projView * model * sourcePosition;

	vec4 texturePos = Texture;
	float frameHeight = texturePos.w / TextureFrames;
	float currentFrame = mod(GlobalTime * 4.0f, TextureFrames);
    currentFrame = trunc(currentFrame);
	texturePos.w = frameHeight;
	texturePos.y = texturePos.y + currentFrame * frameHeight;

    vs_out.UvPosition = UvCoordinates;
    vs_out.Texture = TransformTextureCoord(texturePos,UvCoordinates,TextureLayer);
    vs_out.Color = color;
    vs_out.Light = light;
}
