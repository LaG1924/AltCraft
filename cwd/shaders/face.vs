#version 330 core
layout (location = 0) in vec3 position;
/*layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec4 Texture;
layout (location = 12) in vec3 color;
layout (location = 13) in vec2 light;*/

out VS_OUT {
    vec2 UvPosition;
    flat vec4 Texture;
    flat vec3 Color;
    flat vec2 Light;
    flat int Face;
} vs_out;

uniform  mat4 view;
uniform  mat4 projection;
uniform  mat4 model;

void main()
{
    vec4 sourcePosition = vec4(position,1.0f);
    gl_Position = projection * view * model * sourcePosition;

/*    vs_out.UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    vs_out.Texture = Texture;
    vs_out.Color = color;
    vs_out.Light = light;
    vs_out.Face = gl_VertexID / 6;*/

    vs_out.UvPosition = vec2(0,0);
    vs_out.Texture = vec4(0,0,1,1);
    vs_out.Color = vec3(0,0.2,1.0);
    vs_out.Light = vec2(16,16);
    vs_out.Face = gl_VertexID / 6;
}
