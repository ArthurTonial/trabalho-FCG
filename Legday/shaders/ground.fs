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

uniform sampler2D shadowMap;

in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexNormal; // the input variable from the vertex shader (same name and same type)  
in vec4 vertexPos; // the input variable from the vertex shader (same name and same type)  
in vec4 fragPosLightSpace; // the input variable from the vertex shader (same name and same type)  

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
    vec4 camera_vector = normalize(vertexPos - camera_position);

    vec4 ref_angle = -sun_direction + 2 * vertexNormal * max(0.0, dot(vertexNormal,sun_direction));
    vec4 glossy_factor = glossy_color * pow(max(0.0, dot(ref_angle, camera_vector)), glossy);
    vec4 diffuse_factor = diffuse_color * (max(0.0, dot(sun_direction,vertexNormal)) + 0.01);
    vec4 reflection_factor;

    //FragColor = lerp(light_color, diffuse_color, max(dot(vertexNormal, sun_direction), 0.0)) * shadowTest(fragPosLightSpace);
    FragColor = (diffuse_factor + glossy_factor * 0.3) * shadowTest(fragPosLightSpace);
    //FragColor = (diffuse_factor + glossy_factor * 0.3);
}