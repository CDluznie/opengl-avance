#version 430

layout(local_size_x = 1, local_size_y = 1) in;

layout(rgba32f, binding = 0) uniform readonly image2D uInputImage;
layout(binding = 1) uniform writeonly image2D uOutputImage;

uniform float uAlpha;

void main() {
 
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	vec4 color = uAlpha*imageLoad(uInputImage,pixelCoords);
	imageStore(uOutputImage, pixelCoords, color);

}
