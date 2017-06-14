#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec2 BlockId;
layout (location = 8) in mat4 model;
//layout (location = 12) in something....

out vec2 UvPosition;
out vec3 FragmentPosition;
flat out int Block;
flat out int State;
out vec4 ndcPos;

uniform  mat4 view;
uniform  mat4 projection;
uniform  float time;

void main()
{
    UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    FragmentPosition = position;
    Block = int(BlockId.x);
    State = int(BlockId.y);

    vec4 sourcePosition = vec4(position,1.0f);
    ndcPos = (projection*view*model) * sourcePosition;
    gl_Position = projection * view * model * sourcePosition;

}
