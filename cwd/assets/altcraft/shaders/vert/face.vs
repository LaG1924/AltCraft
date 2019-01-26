#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec4 Texture;
layout (location = 8) in mat4 model;
layout (location = 12) in vec3 color;
layout (location = 13) in vec2 light;
layout (location = 14) in float TextureLayer;

out VS_OUT {
    vec2 UvPosition;
    vec3 Texture;
    vec3 Color;
    vec2 Light;
} vs_out;

//uniform  mat4 view;
//uniform  mat4 projection;
uniform mat4 projView;

vec3 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords, float Layer) {
    float x = TextureAtlasCoords.x;
    float y = TextureAtlasCoords.y;
    float w = TextureAtlasCoords.z;
    float h = TextureAtlasCoords.w;
    vec2 A = vec2(x, 1 - y - h);
    vec2 B = vec2(x + w, 1 - y);
	vec2 transformed = A + UvCoords * (B - A);
    return vec3(transformed.x, transformed.y, Layer);
}

void main()
{
    vec4 sourcePosition = vec4(position,1.0f);
    gl_Position = projView * model * sourcePosition;

    vs_out.UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    vs_out.Texture = TransformTextureCoord(Texture,UvCoordinates,TextureLayer);
    vs_out.Color = color;
    vs_out.Light = light;
}
