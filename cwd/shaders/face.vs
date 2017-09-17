#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec4 Texture;
layout (location = 12) in vec3 color;
layout (location = 13) in vec2 light;

out VS_OUT {
    vec2 UvPosition;
    flat vec4 Texture;
    flat vec3 Color;
    flat vec2 Light;
    flat int Face;
} vs_out;

uniform  mat4 view;
uniform  mat4 projection;

void main()
{
    vec4 sourcePosition = vec4(position,1.0f);
    gl_Position = projection * view * sourcePosition;

    vs_out.UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    vs_out.Texture = Texture;
    vs_out.Color = color;
    vs_out.Light = light;
    vs_out.Face = gl_VertexID / 6;
}
