#version 330 core

in vec2 UvPosition;

uniform int Block;
uniform int State;
uniform sampler2D textureAtlas;
uniform float time;

// TextureIndex: [most significant bit]<-...<-side[3bit]<-id[13]<-state[4]
layout(std140) uniform TextureIndexes { // binding point: 0
  int totalTextures;
  int indexes[1023];
};
layout(std140) uniform TextureData { // binding point: 1
  vec4 textureData[1024];
};

vec4 GetTextureByBlockId();
vec2 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords);
vec4 CheckIndexValidness();
vec4 GetDepthColor();
vec4 GetCheckerColor();
vec4 VTC(int value);

in vec3 FragmentPosition;
int GetBlockSide(){
    int side=6;
      if (FragmentPosition.y==-0.5)
              side=0;
      else if (FragmentPosition.y==0.5)
              side=1;
      else if (FragmentPosition.x==-0.5)
          side = 3;
      else if (FragmentPosition.x==0.5)
          side = 2;
      else if (FragmentPosition.z==-0.5)
          side=4;
      else if (FragmentPosition.z==0.5)
          side=5;
    return side;
}
int index,side,id,state;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
  vec4 BlockTextureCoords = GetTextureByBlockId();
  vec2 AtlasCoords = TransformTextureCoord(BlockTextureCoords, UvPosition);
  gl_FragColor = texture(textureAtlas, AtlasCoords);
  if (id==2 && side==1) { //Grass colorizing
    const float BiomeColor = 0.275;
    vec3 hsvColor = rgb2hsv(gl_FragColor.xyz);
    hsvColor[0]+=BiomeColor;
    hsvColor[1]=0.63;
    hsvColor[2]+=0.1;
    gl_FragColor = vec4(hsv2rgb(hsvColor),1);
  }
}

vec4 GetTextureByBlockId() {
    int BlockSide = GetBlockSide();
    for (int i = 0; i < totalTextures; i++) {
        index = indexes[i];
        side = (index & 0x70000) >> 16;
        id = (index & 0xFF0) >> 4;
        state = index & 0xF;

        if (id != Block)
            continue;
        if (state != State)
            continue;
        if (side == 6)
            return textureData[i];
        if (BlockSide == side)
            return textureData[i];
        if (side == 6)
            return textureData[i];
        else if (side == BlockSide)
            return textureData[i];
    }
  // Fallback TNT texture
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

vec4 CheckIndexValidness() {
  vec4 color = vec4(0, 1, 0, 1);
  if (totalTextures != 6)
    return vec4(1, 0, 0, 1);
  if (indexes[0] != 393248)
    return vec4(1, 1, 0, 1);
  for (int i = 1; i < 20; i++)
    if (indexes[i] != 0)
      return vec4(0, 0, 1, 1);
  return vec4(0, 1, 0, 1);
}

float near = 1.0;
float far = 100.0;

float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC
  return (2.0 * near * far) / (far + near - z * (far - near));
}

vec4 GetDepthColor() {
  float depth =
      LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
  return vec4(vec3(depth), 1.0f);
}

vec4 GetCheckerColor() {
    if (UvPosition.x>0.5 && UvPosition.y<0.5 || UvPosition.x<0.5 && UvPosition.y>0.5)
        return vec4(0.7,0.7,0,1);
    else
        return vec4(0,0,0,1);
}

vec4 VTC(int value){
    switch(value)
    {
        case 0:
            return vec4(0,0,0,1);
        case 1:
            return vec4(1,0,0,1);
        case 2:
            return vec4(0,1,0,1);
        case 3:
            return vec4(0,0,1,1);
        case 4:
            return vec4(1,1,0,1);
        case 5:
            return vec4(1,0,1,1);
        case 6:
            return vec4(0,1,1,1);
    }
    return vec4(1,1,1,1);
}
