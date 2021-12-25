#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D blurInput;
uniform int blurScale;

void main() {
    vec2 texelSize = 1.0f / vec2(textureSize(blurInput, 0));
    vec4 result = vec4(0.0f);
    for (int x = -blurScale; x < blurScale; x++) 
    {
        for (int y = -blurScale; y < blurScale; y++) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(blurInput, uv + offset);
        }
    }
    fragColor = result / pow(blurScale * 2.0f, 2);
}
