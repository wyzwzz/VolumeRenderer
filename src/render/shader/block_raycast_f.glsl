#version 430 core

out vec4 frag_color;

uniform sampler1D transfer_func;
uniform sampler2D preInt_transfer_func;
uniform sampler3D cache_volume0;
uniform sampler3D cache_volume1;
uniform sampler3D cache_volume2;

layout(std430,binding=0) buffer MappingTable{
    uvec4 page_entry[];
}mapping_table;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;

uniform int window_width;
uniform int window_height;
uniform float step;
uniform float view_depth;
uniform vec3 view_pos;
uniform vec3 view_direction;//assert normalized
uniform vec3 view_right;//assert normalized
uniform vec3 view_up;//assert normalized
uniform float view_right_space;//measure in raw volume world
uniform float view_up_space;
uniform vec3 light_direction;//assert normalized
uniform vec4 bg_color;


uniform int block_length;
uniform int padding;
uniform ivec3 block_dim;
uniform ivec3 texture_size3;//size3 for cache_volume012

vec4 virtualSample(vec3 samplePos);

vec3 phongShading(vec3 samplePos,vec3 diffuseColor);

void main()
{
    int x_pos=int(gl_FragCoord.x)-window_width/2;
    int y_pos=int(gl_FragCoord.y)-window_height/2;
    vec3 ray_start_pos=view_pos+x_pos*view_right*view_right_space+y_pos*view_up*view_up_space;
    int steps=int(view_depth/step);

    frag_color=vec4(ray_start_pos,1.f);
//    frag_color=vec4(view_direction,1.f);


}

vec4 virtualSample(vec3 samplePos)
{
    vec4 scalar;



    return scalar;
}

vec3 phongShading(vec3 samplePos,vec3 diffuseColor)
{
    vec3 N;
    #define CUBIC
    #ifdef CUBIC
    float value[27];
    float t1[9];
    float t2[3];
    for(int k=-1;k<2;k++){//z
        for(int j=-1;j<2;j++){//y
            for(int i=-1;i<2;i++){//x
                value[(k+1)*9+(j+1)*3+i+1]=texture(volume_data,samplePos+vec3(voxel*i,voxel*j,voxel*k)).r;
            }
        }
    }
    int x,y,z;
    //for x-direction
    for(z=0;z<3;z++){
        for(y=0;y<3;y++){
            t1[z*3+y]=(value[18+y*3+z]-value[y*3+z])/2;
        }
    }
    for(z=0;z<3;z++)
    t2[z]=(t1[z*3+0]+4*t1[z*3+1]+t1[z*3+2])/6;
    N.x=(t2[0]+t2[1]*4+t2[2])/6;


    //for y-direction
    for(z=0;z<3;z++){
        for(x=0;x<3;x++){
            t1[z*3+x]=(value[x*9+6+z]-value[x*9+z])/2;
        }
    }
    for(z=0;z<3;z++)
    t2[z]=(t1[z*3+0]+4*t1[z*3+1]+t1[z*3+2])/6;
    N.y=(t2[0]+t2[1]*4+t2[2])/6;

    //for z-direction
    for(y=0;y<3;y++){
        for(x=0;x<3;x++){
            t1[y*3+x]=(value[x*9+y*3+2]-value[x*9+y*3])/2;
        }
    }
    for(y=0;y<3;y++)
    t2[y]=(t1[y*3+0]+4*t1[y*3+1]+t1[y*3+2])/6;
    N.z=(t2[0]+t2[1]*4+t2[2])/6;
    #else
    //    N.x=value[14]-value[12];
    //    N.y=value[16]-value[10];
    //    N.z=value[22]-value[4];
    N.x=(texture(volume_data,samplePos+vec3(step,0,0)).r-texture(volume_data,samplePos+vec3(-step,0,0)).r);
    N.y=(texture(volume_data,samplePos+vec3(0,step,0)).r-texture(volume_data,samplePos+vec3(0,-step,0)).r);
    N.z=(texture(volume_data,samplePos+vec3(0,0,step)).r-texture(volume_data,samplePos+vec3(0,0,-step)).r);
    #endif

    N=-normalize(N);

    vec3 L=-light_direction;
    vec3 R=L;//-ray_direction;

    vec3 ambient=ka*diffuseColor.rgb;
    vec3 specular=ks*pow(max(dot(N,(L+R)/2.0),0.0),shininess)*vec3(1.0f);
    vec3 diffuse=kd*max(dot(N,L),0.0)*diffuseColor.rgb;
    return ambient+specular+diffuse;
}