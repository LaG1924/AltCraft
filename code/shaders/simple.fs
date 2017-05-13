#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D blockTexture;
uniform int block;
uniform float time;

void main()
{
    //color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.0);
    /*if (block==1)
        color = vec4(0.2,0.2,0.2,1);
    else if (block==0)
        color = vec4(0,0,0,1);
    else
        color = vec4(1,1,1,1);*/
    color = texture(blockTexture,TexCoord);
    //color = vec4(TexCoord.x,TexCoord.y,0,1);
}

