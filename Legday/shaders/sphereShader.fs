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
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;

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

vec4 saturate(vec4 a){
    return min(vec4(1.0), max(vec4(0.0), a));
}

void main()
{
    vec4 object_origin = mode_m * vec4(0.0,0.0,0.0,1.0);
    vec3 n = normalize(vertexNormal).xyz;
    vec3 v = normalize(camera_position - vertexPos).xyz;

    vec3 r = normalize(cross(v, vec3(0.0,1.0,0.0)));
    vec3 u = normalize(cross(v, r));

    vec3 l = -sun_direction.xyz;

    vec3 ref = -l + 2*n*dot(n, l);

    vec2 customUV = vec2(dot(n,r)+0.5, -1*dot(n,u)+0.5);

    float customU = atan(customUV.r - 0.5, customUV.g - 0.5).r / 6.283 + 0.5;

    vec2 customUV2 = vec2(customU, pow(dot(n, v),2));

    float t = time - round(time);
    float t2 = time * 0.25 - round(time * 0.25);

    vec4 center_color = vec4(1.00, 0.0200, 0.281, 1.0);
    vec4 edge_color1 = vec4(0.560, 0.00, 0.960, 1.0);
    vec4 edge_color2 = vec4(0.430, 0.00, 0.394, 1.0);

    vec4 edge = lerp(edge_color1, edge_color2, dot(n, v) * 28.0 - 29.7 * texture(TextureImage3, customUV2 * vec2(2.0, 0.5) + vec2(0.0, t2)).g);
    vec4 center = lerp(edge_color2 * 0.5, center_color, pow(dot(n, v), 14));
    vec4 base = lerp(edge, center, pow(dot(n, v), 1));
    vec4 whisps = center_color;
    vec4 unlit = lerp(base, whisps, texture(TextureImage3, customUV2 + vec2(0.0,t)).r);

    //vec2 customUV = vec2(0.0);
    
    FragColor = unlit + vec4(2.0) * pow(max(0.0,dot(ref,v)), 12.0) * shadowTest(fragPosLightSpace);
     if(selected > 0.5f){
        FragColor = vertexColor;
    }
}