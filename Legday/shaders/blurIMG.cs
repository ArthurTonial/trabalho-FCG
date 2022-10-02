#version 460 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in; // the position variable has attribute position 0
layout (rgba32f, binding = 0) uniform image2D screen;

#define KERNEL 3
#define alpha 1.5
#define PI 3.14159265359

//float ker[KERNEL][KERNEL];

// float ker[3][3] = 
// {
//     {0.0,       0.015625,   0.0},
//     {0.015625,  0.9375,     0.015625},
//     {0.0,       0.015625,   0.0}
// };

float ker[KERNEL][KERNEL] = 
{
    {0.0,       0.0625,   0.0},
    {0.0625,  0.5,     0.0625},
    {0.0,       0.0625,   0.0}
};

void main()
{
    // for(int i = 0; i < KERNEL; i++){
    //     for(int j = 0; j < KERNEL; j++){
    //         int x = -KERNEL/2 + j;
    //         int y = -KERNEL/2 + i;
    //         ker[i][j] = (1.0 / (alpha*alpha*2*PI)) * exp(-(x*x+y*y)/(2*alpha*alpha));
    //     }
    // }

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    ivec2 dims = imageSize(screen);
    

    vec4 pixel_value = vec4(0.0,0.0,0.0,0.0);

    
    for(int i = -KERNEL/2; i <= KERNEL/2; i++){
        for(int j = -KERNEL/2; j <= KERNEL/2; j++){
            pixel_value += imageLoad(screen, pixel_coords + ivec2(i, j)) * ker[i+KERNEL/2][j+KERNEL/2];
        }
    }
    
    //pixel_value = imageLoad(screen, pixel_coords);

    imageStore(screen, pixel_coords, pixel_value);
    //imageStore(screen, pixel_coords, pixel_value * vec4( (1.0 - RMV_AMNT*4.5), (1.0 - RMV_AMNT*2.5), (1.0 - RMV_AMNT),1.0));
}
