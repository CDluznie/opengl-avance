#version 330 core

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fColor;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

vec3 blinnPhong(vec3 lightDir, vec3 lightIntensity) {
	vec3 wi = normalize(lightDir);
	vec3 w0 = normalize(-vViewSpacePosition);
	vec3 halfVector = (w0 + wi)/2.;
	vec3 Ka = 0.15 * texture(uKaSampler, vTexCoords).xyz * uKa;
	vec3 Kd = texture(uKdSampler, vTexCoords).xyz * uKd;
	vec3 Ks = texture(uKsSampler, vTexCoords).xyz * uKs;
	float shininess = texture(uShininessSampler, vTexCoords).x * uShininess;
	return lightIntensity*(Kd*(max(0., dot(wi,vViewSpaceNormal))) + Ks*(pow(max(0., dot(halfVector,vViewSpaceNormal)), shininess))) + Ka;
}

void main() {

	float distToPointLight = length(uPointLightPosition - vViewSpacePosition);
	vec3 dirToPointLight = uPointLightPosition - vViewSpacePosition;
	vec3 intensityToPointLight = (uPointLightIntensity) / (distToPointLight*distToPointLight);
	fColor = (blinnPhong(uDirectionalLightDir, uDirectionalLightIntensity) + blinnPhong(dirToPointLight, intensityToPointLight));

}
