#version 330 core
in vec2 TexCoord;

uniform sampler2D textureAtlas;
uniform int block;

vec4 GetTextureByBlockId(int BlockId) {
    return vec4(0,0,0,0);
}

void main()
{
    vec4 TextureCoords = GetTextureByBlockId(block);
    gl_FragmentColor = texture(blockTexture,TexCoord);
}

