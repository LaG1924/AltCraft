#version 330 core

in gl_Vertex
{
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
} gl_in[];

in VS_OUT {
    vec2 UvPosition;
    vec3 FragmentPosition;
    flat int Block;
    flat int State;
    vec4 ndcPos;
} gs_in[];

out GS_OUT {
    vec2 UvPosition;
    vec3 FragmentPosition;
    flat int Block;
    flat int State;
    vec4 ndcPos;
} gs_out[];

void main() {
    gs_out[0].UvPosition = gs_in[0].UvPosition;
    gs_out[0].FragmentPosition = gs_in[0].FragmentPosition;
    gs_out[0].Block = gs_in[0].Block;
    gs_out[0].State = gs_in[0].State;
    gs_out[0].ndcPos = gs_in[0].ndcPos;
}