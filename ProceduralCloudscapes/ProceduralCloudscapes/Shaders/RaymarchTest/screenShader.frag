#version 460 core
#define MAX_STEPS 100
#define MAX_DIST 100.0f
#define SURF_DIST 0.01f

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec3 cameraPos;
uniform vec3 lookAt;
uniform float zoom;

struct ray {
    vec3 pos;
    vec3 dir;
};

ray create_camera_ray(vec2 uv, vec3 camPos, vec3 lookAt, float zoom){
    vec3 f = normalize(lookAt - camPos);
    vec3 r = cross(vec3(0.0,1.0,0.0),f);
    vec3 u = cross(f,r);
    vec3 c=camPos+f*zoom;
    vec3 i=c+uv.x*r+uv.y*u;
    vec3 dir=i-camPos;
    return ray(camPos,dir);
}

float GetDist(vec3 p) {
    vec4 s = vec4(0, 1, 6, 1);
    float sphereDist = length(p-s.xyz) - s.w;
    float planeDist = p.y;
    float d = min(sphereDist, planeDist);
    return d;
}

float RayMarch(vec3 ro, vec3 rd) {
    float d0=0.;
    for (int i=0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd*d0;
        float dS = GetDist(p);
        d0 += dS;
        if (d0>MAX_DIST || dS<SURF_DIST) break;
    }
    return d0;
}

vec3 GetNormal(vec3 p) {
    float d = GetDist(p);
    vec2 e = vec2(0.01, 0);
    vec3 n = d - vec3(
        GetDist(p - e.xyy),
        GetDist(p - e.yxy),
        GetDist(p - e.yyx));
    return normalize(n);
}

float GetLight(vec3 p) {
    vec3 lightPos = vec3(0, 5, 6);
    vec3 l = normalize(lightPos - p);
    vec3 n = GetNormal(p);
    float diff = clamp(dot(n, l), 0., 1.);
    float d = RayMarch(p+n*SURF_DIST, l);
    if (d<length(lightPos-p)) diff *= .1;
    return diff;
}

void main()
{
//    vec3 col = texture(screenTexture, TexCoords).rgb;

    vec2 iResolution = vec2(800.0, 600.0);
    vec2 uv = (gl_FragCoord.xy-0.5*iResolution.xy)/iResolution.y;
    vec3 col = vec3(0);

//    vec3 ro = vec3(0, 2, 0);
    ray ray = create_camera_ray(uv, cameraPos, lookAt, zoom);
    vec3 ro = ray.pos;
    vec3 rd = normalize(vec3(uv.x, uv.y, 1));
//    vec3 rd = ray.dir;
    float d = RayMarch(ro, rd);
    vec3 p = ro + rd * d;

    float diff = GetLight(p);
    col = vec3(diff);
    FragColor = vec4(col, 1.0);
}