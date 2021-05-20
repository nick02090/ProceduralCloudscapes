#version 460 core

// Define the number of CPs in the output patch
layout ( vertices = 3 ) out;

// Tessellation levels thresholds
#define TESS_LEVEL_THRESHOLD_LOW 1024.0
#define TESS_LEVEL_THRESHOLD_MEDIUM 2048.0
#define TESS_LEVEL_THRESHOLD_HIGH 4096.0
#define TESS_LEVEL_THRESHOLD_VERY_HIGH 8192.0

// Tessellation levels
#define TESS_LEVEL_ZERO 1.0
#define TESS_LEVEL_VERY_LOW 8.0
#define TESS_LEVEL_LOW 16.0
#define TESS_LEVEL_MEDIUM 32.0
#define TESS_LEVEL_HIGH 64.0

uniform vec3 cameraPosition;

// INPUT
in vec3 WorldPos_TECS_in[];
in vec3 Normal_TECS_in[];
in vec2 TexCoord_TECS_in[];

// OUTPUT
out vec3 WorldPos_TESS_in[];
out vec3 Normal_TESS_in[];
out vec2 TexCoord_TESS_in[];

// Calculates the tessellation level based on the average distance
float calculateTessellationLevel(vec2 distances) {
	// Calculate the average distance of given distances
	float AverageDistance = (distances.x + distances.y) / 2.0;

	// Calculate the tessellation level
	if (AverageDistance <= TESS_LEVEL_THRESHOLD_LOW) {
		return TESS_LEVEL_HIGH;
	} else if (AverageDistance <= TESS_LEVEL_THRESHOLD_MEDIUM) {
		return TESS_LEVEL_MEDIUM;
	} else if (AverageDistance <= TESS_LEVEL_THRESHOLD_HIGH) {
		return TESS_LEVEL_LOW;
	} else if (AverageDistance <= TESS_LEVEL_THRESHOLD_VERY_HIGH) {
		return TESS_LEVEL_VERY_LOW;
	} else {
		return TESS_LEVEL_ZERO;
	}
}

void main()
{
	// Set the control points of the output patch
	WorldPos_TESS_in[gl_InvocationID] = WorldPos_TECS_in[gl_InvocationID];
	Normal_TESS_in[gl_InvocationID] = Normal_TECS_in[gl_InvocationID];
	TexCoord_TESS_in[gl_InvocationID] = TexCoord_TECS_in[gl_InvocationID];

	// Calculate the distance from the camera to the 3 control points
	vec3 cameraToVertexDistance = vec3(distance(cameraPosition, WorldPos_TESS_in[0]),
										distance(cameraPosition, WorldPos_TESS_in[1]),
										distance(cameraPosition, WorldPos_TESS_in[2]));

	// Calculate tessellation levels (number of segments on each edge)
	gl_TessLevelOuter[0] = calculateTessellationLevel(cameraToVertexDistance.yz);
	gl_TessLevelOuter[1] = calculateTessellationLevel(cameraToVertexDistance.zx);
	gl_TessLevelOuter[2] = calculateTessellationLevel(cameraToVertexDistance.xy);

	// Calculate tessellation levels (how many rings the triangle will contain)
	gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}