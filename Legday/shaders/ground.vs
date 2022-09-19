#version 460 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec4 vNormal; // the color variable has attribute position 1

uniform mat4 mode_m;
uniform mat4 view_m;
uniform mat4 proj_m;

uniform mat4 sun_view_m;
uniform mat4 sun_proj_m;

#define PI 3.14159265359
#define f 100
#define n 0.1

out vec4 vertexColor; // specify a color output to the fragment shader
out vec4 vertexNormal; // specify a color output to the fragment shader
out vec4 vertexPos; // specify a color output to the fragment shader
out vec4 fragPosLightSpace; // specify a color output to the fragment shader

void main()
{
    // model matrix

    // view matrix

    // projection matrix


    gl_Position = proj_m * view_m * mode_m * vec4(aPos,1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = vec4(1.0, 1.0, 0.0, 1.0); // set the output variable to a dark-red color
    vertexNormal = normalize(mode_m * vNormal);
    vertexPos = mode_m * vec4(aPos,1.0);
    fragPosLightSpace = sun_proj_m * sun_view_m * vertexPos;
}
