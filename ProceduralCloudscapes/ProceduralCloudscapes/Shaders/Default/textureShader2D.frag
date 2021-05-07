#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform int channel;

void main()
{    
    vec4 tex = texture(texture1, TexCoords);
    float col;
    if (channel == 0) {
        col = tex.r;
    } else if (channel == 1) {
        col = tex.g;
    } else if (channel == 2) {
        col = tex.b;
    } else {
        col = tex.a;
    }
    FragColor = vec4(vec3(col), 1.0);
}