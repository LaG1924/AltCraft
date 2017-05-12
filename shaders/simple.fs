#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.0);
}