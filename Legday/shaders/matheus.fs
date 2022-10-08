#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 texcoords; // the input variable from the vertex shader (same name and same type)  

uniform sampler2D TextureImage9;
uniform sampler2D TextureImage10;

uniform float power;

void main()
{
    vec4 img1 = texture(TextureImage9, texcoords);
    FragColor = vec4(img1.rgb, img1.w);
}