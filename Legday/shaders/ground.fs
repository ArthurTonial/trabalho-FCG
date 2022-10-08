#version 460 core

layout(location = 0) out vec4 FragColor;

// set in material
uniform float time;

uniform float diffuse;
uniform float glossy;
uniform float metallic;
uniform vec4 diffuse_color;
uniform vec4 glossy_color;
uniform float selected;

uniform mat4 mode_m;

// set in renderer
uniform vec4 light_color;
uniform vec4 camera_position;
uniform vec4 camera_direction;
uniform vec4 sun_position;
uniform vec4 sun_direction;

uniform sampler2D shadowMap;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;

in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexNormal; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexPos; // the input variable from the vertex shader (same name and same type)  
in vec4 fragPosLightSpace; // the input variable from the vertex shader (same name and same type)  
in vec2 texcoords; // the input variable from the vertex shader (same name and same type)  

vec4 lerp(vec4 a, vec4 b, float alpha){

    return ((b - a) * alpha) + a;
}

float lerp(float a, float b, float alpha){

    return ((b - a) * alpha) + a;
}

float shadowTest(vec4 fPosLSpace){
    float bias = max(0.008 * (1.0 - dot(vertexNormal, -sun_direction)), 0.005);  

    vec3 projCoords = fPosLSpace.xyz / fPosLSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    if(projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0) return 1.0; 

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 0.0 : 1.0;

    return shadow;
}

void main()
{
    vec4 l = -sun_direction;
    vec4 n = normalize(vertexNormal);
    vec4 p = vertexPos;
    vec4 v = normalize(camera_position - p);

    vec4 r = -l + 2*n*dot(n, l);

    vec2 customUV = texcoords * 10.0f;

    float q = 30.0f;

    vec4 Kd = texture(TextureImage4, customUV);
    vec4 Ks = vec4(1.0,1.0,1.0,1.0);
    vec4 Ka = vec4(0.0,0.0,0.0,1.0);

    vec4 I = light_color;
    vec4 Ia = vec4(0.0,0.0,0.0,1.0);

    vec4 glossy_factor =  Ks * I * pow(max(0.0,dot(r,v)), q) * shadowTest(fragPosLightSpace);
    vec4 diffuse_factor = Kd * I * (max(0.3, 1.0+dot(n, l))) * max(0.2,shadowTest(fragPosLightSpace));
    vec4 ambient_factor = Ka * Ia; 
    vec4 reflection_factor;

    //diffuse_factor = diffuse_color * pow(max(0.0, dot(vertexNormal,camera_vector)), 3);

    FragColor = diffuse_factor + glossy_factor + ambient_factor;
    if(selected > 0.5f){
        FragColor = vertexColor;
    }
}