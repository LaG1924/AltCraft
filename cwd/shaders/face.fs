#version 330 core

in VS_OUT {
    vec2 UvPosition;
    flat vec4 Texture;
    flat vec3 Color;
    flat vec2 Light;
    flat int Face;
} fs_in;

uniform sampler2D textureAtlas;
uniform vec2 windowSize;

vec2 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords) {
    float x = TextureAtlasCoords.x;
    float y = TextureAtlasCoords.y;
    float w = TextureAtlasCoords.z;
    float h = TextureAtlasCoords.w;
    vec2 A = vec2(x, 1 - y - h);
    vec2 B = vec2(x + w, 1 - y);
    return A + UvCoords * (B - A);
}

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
//    gl_FragColor = vec4(fs_in.Face / 1000.0f, fs_in.Face / 1000.0f, fs_in.Face / 1000.0f, 1.0f);
    gl_FragColor = vec4(fs_in.UvPosition.xy,0.0f,1.0f);
    return;

    gl_FragColor = texture(textureAtlas,TransformTextureCoord(fs_in.Texture,fs_in.UvPosition));
    if (gl_FragColor.a < 0.3)
        discard;
    vec3 hsvColor = rgb2hsv(gl_FragColor.xyz);
    hsvColor+=fs_in.Color;
    gl_FragColor = vec4(hsv2rgb(hsvColor),1);
    //float faceLight = clamp((fs_in.Light.x + fs_in.Light.y) / 15.0,0.2,1.0);
    //gl_FragColor = vec4(gl_FragColor.rgb * faceLight,gl_FragColor.a);
}
