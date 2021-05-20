#version 460 core
//===============================================================================================
// CONSTANTS
//===============================================================================================

// Domain: triangles
// Edge subdivision method: equal_spacing
// PG will emit triangles in ccw (counter-clockwise) order
layout ( triangles, equal_spacing, ccw ) in;

//===============================================================================================
// STRUCTS
//===============================================================================================

struct fbm {
	float amplitude;
	float frequency;
	int octaves;
	float lacunarity;
	float gain;
	vec2 seed;
	float power;
};

//===============================================================================================
// INPUT 
//===============================================================================================

// INPUT
in vec3 WorldPos_TESS_in[];
in vec3 Normal_TESS_in[];
in vec2 TexCoord_TESS_in[];

// OUTPUT
out vec3 WorldPos_FS_in;
out vec3 Normal_FS_in;
out vec2 TexCoord_FS_in;

// Terrain noise parameters
uniform fbm terrainNoise;
// ProjectionMat * ViewMat (clip space)
uniform mat4 gVP;

//===============================================================================================
// METHODS
//===============================================================================================

// Calculates a random value based on 2D vector (co) and a seed
float rand(vec2 co, vec2 seed){
    return fract(sin(dot(co, vec2(12.9898, 78.233) + seed)) * 43758.5453);
}

// Interpolate between a trio of 2D vectors using 'gl_TessCoord' as a weight
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

// Interpolate between a trio of 3D vectors using 'gl_TessCoord' as a weight
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2) {
	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

// Calculates interpolated noise
float interpolatedNoise(vec2 x, vec2 seed) {
    // Grid
    vec2 p = floor(x);
    vec2 w = fract(x);
	
    // Quintic interpolant
    vec2 u = w * w * w * (w * (w * 6.0 - 15.0) + 10.0);

	// Gradients
	float ga = rand(p + vec2(0.0, 0.0), seed);
	float gb = rand(p + vec2(1.0, 0.0), seed);
	float gc = rand(p + vec2(0.0, 1.0), seed);
	float gd = rand(p + vec2(1.0, 1.0), seed);

	// Interpolation
	return ga + 
			u.x * (gb - ga) + 
			u.y * (gc - ga) + 
			u.x * u.y * (gd - gc - gb + ga);
}

// Calculates fBm for the noise defined with fbm at coord
float noiseFBM(vec2 coord, fbm fbm) {
    // Initial values
    float frequency = fbm.frequency;
    float amplitude = fbm.amplitude;
    float noise = 0.0f;
    // Loop through octaves
    for (int i = 0; i < fbm.octaves; ++i) {
        // Initial value for this noise
        float noiseRes = interpolatedNoise(coord * frequency, fbm.seed);
        // Accumulate noise
        noise += amplitude * noiseRes;
        // Update properties
        frequency *= fbm.lacunarity;
        amplitude *= fbm.gain;
    }
    // Return final fBm noise
    return pow(noise, fbm.power);
}

void main()
{
	// Interpolate the attributes of the output vertex using the barycentric coordinates
	WorldPos_FS_in = interpolate3D(WorldPos_TESS_in[0], WorldPos_TESS_in[1], WorldPos_TESS_in[2]);
	Normal_FS_in = normalize(interpolate3D(Normal_TESS_in[0], Normal_TESS_in[1], Normal_TESS_in[2]));
	TexCoord_FS_in = interpolate2D(TexCoord_TESS_in[0], TexCoord_TESS_in[1], TexCoord_TESS_in[2]);

	// Displace the vertex along the normal
	float displacement = noiseFBM(WorldPos_FS_in.xz, terrainNoise);
	WorldPos_FS_in += Normal_FS_in * displacement;
    gl_Position = gVP * vec4(WorldPos_FS_in, 1.0);
}