#version 330 core

in vec2 UvPosition;

uniform float time;
uniform int block;
uniform sampler2D textureAtlas;

// TextureIndex: [most significant bit]<-...<-side[3bit]<-id[13]<-state[4]
layout(std140) uniform TextureIndexes { // binding point: 0
  int totalTextures;
  int indexes[2047];
};
// layout(std140) uniform TextureData { vec4 textureData[1024]; }; //binding point: 1
// layout(std140) uniform TextureData2 { vec4 textureData2[1024]; }; //binddingpoint: 2
vec4 GetTextureByBlockId(int BlockId);
vec2 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords);

void main() {
vec4 bad = vec4(1,0,0,1);
  vec4 color = vec4(0,1,0,1);
    if (totalTextures!=6)
        color = bad;
    if (indexes[0]!=393248)
        color = bad;
  for (int i=1;i<2047;i++)
    if (indexes[i]!=0)
        color=bad;
  gl_FragColor = color;
  /*vec4 BlockTextureCoords = GetTextureByBlockId(block);
  vec2 AtlasCoords = TransformTextureCoord(BlockTextureCoords, UvPosition);
  gl_FragColor = texture(textureAtlas, AtlasCoords);*/
}

vec4 GetTextureByBlockId(int BlockId) {
  if (indexes[0] == 0)
    return vec4(0.05, 0.004, 0.007, 0.004);

  if (totalTextures == 0)
    return vec4(0, 0, 0.1, 0.1);
  for (int i = 0; i < totalTextures; i++) {
    int index = indexes[i + 1];
    int side = (index & 0xE0000) >> 16;
    int id = (index & 0xFF0) >> 4;
    int state = index & 0xF;
    if (id == BlockId) {
      return vec4(i, 0, 1, 1);
    }
  }
  /*
    TNT texture:
    X 0.0546875
    Y ~0.00442477876106194690
    W 0.0078125
    H ~0.00442477876106194690
  */
  return vec4(0.0546875, 0.00442477876106194690, 0.0078125,
              0.00442477876106194690);
}

vec2 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords) {
  float x = TextureAtlasCoords.x;
  float y = TextureAtlasCoords.y;
  float w = TextureAtlasCoords.z;
  float h = TextureAtlasCoords.w;
  vec2 A = vec2(x, y);
  vec2 B = vec2(x + w, y + h);

  const bool isTextureFlippedVertically = true;
  if (isTextureFlippedVertically) {
    y = 1 - y;
    A = vec2(x, y - h);
    B = vec2(x + w, y);
  }
  return A + UvCoords * (B - A);
}

/*

float near = 1.0;
float far  = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    gl_FragColor = vec4(vec3(depth), 1.0f);
}*/


