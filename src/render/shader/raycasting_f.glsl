#version 430 core
out vec4 frag_color;
layout(binding=0,rgba32f) uniform image2D entry_pos;
layout(binding=1,rgba32f) uniform image2D exit_pos;
uniform sampler1D transfer_func;
uniform sampler2D preInt_transferfunc;
uniform sampler3D volume_data;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 light_direction;

uniform float step;

vec3 ray_direction;

vec3 phongShading(vec3 samplePos,vec3 diffuseColor)
{
    vec3 N;
    N.x=(texture(volume_data,samplePos+vec3(step,0,0)).r-texture(volume_data,samplePos+vec3(-step,0,0)).r);
    N.y=(texture(volume_data,samplePos+vec3(0,step,0)).r-texture(volume_data,samplePos+vec3(0,-step,0)).r);
    N.z=(texture(volume_data,samplePos+vec3(0,0,step)).r-texture(volume_data,samplePos+vec3(0,0,-step)).r);

    N=-normalize(N);

    vec3 L=-light_direction;
    vec3 R=L;//-ray_direction;

    vec3 ambient=ka*diffuseColor.rgb;
    vec3 specular=ks*pow(max(dot(N,(L+R)/2.0),0.0),shininess)*vec3(1.0f);
    vec3 diffuse=kd*max(dot(N,L),0.0)*diffuseColor.rgb;
    return ambient+specular+diffuse;
}
void main()
{
    vec3 start_pos=imageLoad(entry_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 end_pos=imageLoad(exit_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 start2end=end_pos-start_pos;
    ray_direction=normalize(start2end);
    float distance=dot(ray_direction,start2end);
    int steps=int(distance/step);
    vec4 color=vec4(0.0f);
    vec3 simple_pos=start_pos;

    for(int i=0;i<steps;i++){
        vec4 scalar=texture(volume_data,simple_pos);
        vec4 simple_color=texture(transfer_func,scalar.r);

        simple_color.rgb=phongShading(simple_pos,simple_color.rgb);
        color = color + simple_color * vec4(simple_color.aaa, 1.0) * (1.0 - color.a);
        if(color.a>0.99f)
            break;
        simple_pos+=ray_direction*step;
    }

    if(color.a==0.0f)
        discard;
    color=(1-color.a)*vec4(0.0f,0.0f,0.0f,1.0f)+color*color.a;
    frag_color=color;
}