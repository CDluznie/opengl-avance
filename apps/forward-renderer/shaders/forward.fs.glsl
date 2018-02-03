#version 430 

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fColor;

uniform int uDirectionalLightNumber;
uniform int uPointLightNumber;

layout (std430, binding=1) buffer LightInfos { 
	vec4 directional_light_dirs[40];
	vec4 directional_light_intensities[40];
	vec4 point_light_positions[40];
	vec4 point_light_intensities[40];
};

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

	// Directional lights

	vec3 CumulatedDirectionalLight = vec3(0);
	
	for (int i = 0; i < uDirectionalLightNumber; i++) {
		vec3 DirectionalLightDir = directional_light_dirs[i].xyz;
		vec3 DirectionalLightIntensity = directional_light_intensities[i].xyz;
		CumulatedDirectionalLight += blinnPhong(DirectionalLightDir, DirectionalLightIntensity);
	}
	
	// Point lights
	
	vec3 CumulatedPointLight = vec3(0);
	
	for (int i = 0; i < uPointLightNumber; i++) {
		vec3 PointLightPos = point_light_positions[i].xyz;
		vec3 PointLightIntensity = point_light_intensities[i].xyz;
		float distToPointLight = length(PointLightPos - vViewSpacePosition);
		vec3 dirToPointLight = PointLightPos - vViewSpacePosition;
		vec3 intensityToPointLight = (PointLightIntensity) / (distToPointLight*distToPointLight);
		CumulatedPointLight += blinnPhong(dirToPointLight, intensityToPointLight);
	}
	
	fColor = CumulatedDirectionalLight + CumulatedPointLight;

}
