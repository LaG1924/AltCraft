#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D blockTexture;
uniform int block;
uniform float time;

void main()
{
    color = texture(blockTexture,TexCoord);
    //color = vec4(TexCoord.x,TexCoord.y,0.0,1.0);
}

