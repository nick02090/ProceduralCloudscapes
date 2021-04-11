#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler3D texture1;

void main()
{
    vec4 tex = textureLod(texture1, vec3(TexCoords, 1.0), 0);
    float r = tex.x;
    vec3 col = vec3(0.0);
    col += vec3(r);
    FragColor = vec4(col, 1.0);
}