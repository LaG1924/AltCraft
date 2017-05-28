#version 330 core

struct TextureCoordData {
    int blockId;
    int blockState;
    int blockSide;
    vec4 texture;
};

in vec2 UvPosition;

uniform sampler2D textureAtlas;
uniform int block;

vec4 GetTextureByBlockId(int BlockId) {
    return vec4(0,0,0,0);
}

vec4 TransformTextureCoord(vec4 TextureAtlasCoords){
    return vec4(0,0,0,0);
}

void main()
{
    vec4 TextureCoords = GetTextureByBlockId(block);
    gl_FragColor = texture(textureAtlas,UvPosition);
}

