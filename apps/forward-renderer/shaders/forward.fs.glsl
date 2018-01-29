#version 330 core

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec3 vTexCoords;

out vec3 fColor;

void main() {
	
	fColor = normalize(vViewSpaceNormal);
	
}
