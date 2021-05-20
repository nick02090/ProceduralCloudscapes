#version 460 core
layout ( location = 0 ) in vec3 Position_VS_in;
layout ( location = 1 ) in vec3 Normal_VS_in;
layout ( location = 2 ) in vec2 TexCoord_VS_in;
layout ( location = 3 ) in vec2 aPosition;

out vec3 WorldPos_TECS_in;
out vec2 TexCoord_TECS_in;
out vec3 Normal_TECS_in;

void main()
{
	WorldPos_TECS_in = Position_VS_in;
	WorldPos_TECS_in.xz += aPosition;
	Normal_TECS_in = Normal_VS_in;
	TexCoord_TECS_in = TexCoord_VS_in;
}