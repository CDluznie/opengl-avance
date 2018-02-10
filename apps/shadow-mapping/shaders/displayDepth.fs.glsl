#version 430

uniform sampler2D uGDepth;

out vec3 fColor;

void main() {

    float depth = texelFetch(uGDepth, ivec2(gl_FragCoord.xy), 0).r;
    fColor = vec3(pow(depth, 1/10.f));

}