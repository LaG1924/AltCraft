#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec2 BlockId;
layout (location = 8) in mat4 model;
//layout (location = 12) in something....

out VS_OUT {
    vec2 UvPosition;
    vec3 FragmentPosition;
    flat int Block;
    flat int State;
    vec4 ndcPos;
} vs_out;

uniform  mat4 view;
uniform  mat4 projection;
uniform  float time;

void main()
{
    vs_out.UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    vs_out.FragmentPosition = position;
    vs_out.Block = int(BlockId.x);
    vs_out.State = int(BlockId.y);

    vec4 sourcePosition = vec4(position,1.0f);
    vs_out.ndcPos = (projection*view*model) * sourcePosition;
    gl_Position = projection * view * model * sourcePosition;

}
