#version 460 core

layout(location = 0) out vec4 FragColor;

// set in material
uniform float diffuse;
uniform float glossy;
uniform float metallic;
uniform vec4 diffuse_color;
uniform vec4 glossy_color;

// set in renderer
uniform vec4 light_color;
uniform vec4 camera_position;
uniform vec4 camera_direction;
uniform vec4 sun_position;
uniform vec4 sun_direction;
uniform mat4 sun_view_m;
uniform mat4 sun_proj_m;

in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexNormal; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexPos; // the input variable from the vertex shader (same name and same type)  

vec4 lerp(vec4 a, vec4 b, float alpha){

    return ((b - a) * alpha) + a;
}

void main()
{
    FragColor = diffuse_color;
    //FragColor = vec4(vertexNormal.xyz, 1.0);
}