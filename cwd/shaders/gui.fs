#version 330 core

in vec2 uv;

uniform vec4 widgetTexture;
uniform sampler2D textureAtlas;

vec2 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords) {
    float x = TextureAtlasCoords.x;
    float y = TextureAtlasCoords.y;
    float w = TextureAtlasCoords.z;
    float h = TextureAtlasCoords.w;
    vec2 A = vec2(x, 1 - y - h);
    vec2 B = vec2(x + w, 1 - y);
    return A + UvCoords * (B - A);
}

void main(){
    vec4 color = texture(textureAtlas,TransformTextureCoord(widgetTexture,uv));
    gl_FragColor = color;
}