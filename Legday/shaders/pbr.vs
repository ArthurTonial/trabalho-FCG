#version 460 core
layout (location = 0) in vec4 aPos; // the position variable has attribute position 0
layout (location = 1) in vec4 vNormal; // the color variable has attribute position 1
layout (location = 2) in vec2 uv; // the color variable has attribute position 1

uniform mat4 mode_m;
uniform mat4 view_m;
uniform mat4 proj_m;

uniform mat4 sun_view_m;
uniform mat4 sun_proj_m;

// set in material
uniform float time;

uniform float diffuse;
uniform float glossy;
uniform float metallic;
uniform vec4 diffuse_color;
uniform vec4 glossy_color;
uniform float selected;

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

out vec4 vertexColor; // specify a color output to the fragment shader
out vec4 vertexNormal; // specify a color output to the fragment shader
out vec4 vertexPos; // specify a color output to the fragment shader
out vec4 fragPosLightSpace; // specify a color output to the fragment shader
out vec2 texcoords; // specify a color output to the fragment shader

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

    gl_Position = proj_m * view_m * mode_m * aPos; // see how we directly give a vec3 to vec4's constructor
    vertexColor = vec4(1.0, 1.0, 0.0, 1.0); // set the output variable to a dark-red color
    vertexNormal = inverse(transpose(mode_m)) * vNormal;
    vertexNormal.w = 0.0;
    vertexPos = mode_m * aPos;
    texcoords = uv;
    fragPosLightSpace = sun_proj_m * sun_view_m * vertexPos;

    vec4 l = -sun_direction;
    vec4 n = normalize(vertexNormal);
    vec4 p = vertexPos;
    vec4 v = normalize(camera_position - p);

    vec4 r = -l + 2*n*dot(n, l);

    float q = texture(TextureImage2, texcoords).r;

    vec4 Kd = texture(TextureImage1, texcoords);
    vec4 Ks = vec4(1.0,1.0,1.0,1.0);
    vec4 Ka = vec4(0.0,0.0,0.0,1.0);

    vec4 I = light_color;
    vec4 Ia = vec4(0.0,0.0,0.0,1.0);

    vec4 glossy_factor =  Ks * I * pow(max(0.0,dot(r,v)), q * 32.0) * shadowTest(fragPosLightSpace);
    vec4 diffuse_factor = Kd * I * (max(0.3, dot(n, l))) * max(0.2,shadowTest(fragPosLightSpace));
    vec4 ambient_factor = Ka * Ia; 
    vec4 reflection_factor;

    //diffuse_factor = diffuse_color * pow(max(0.0, dot(vertexNormal,camera_vector)), 3);

    vertexColor = diffuse_factor + glossy_factor + ambient_factor;

}
