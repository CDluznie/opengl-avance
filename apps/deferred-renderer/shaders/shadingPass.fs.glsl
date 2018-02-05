#version 430 

out vec3 fColor;

uniform int uDirectionalLightNumber;
uniform int uPointLightNumber;

layout (std430, binding=1) buffer LightInfos { 
	vec4 directional_light_dirs[40];
	vec4 directional_light_intensities[40];
	vec4 point_light_positions[40];
	vec4 point_light_intensities[40];
};

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGGlossyShininess;

vec3 blinnPhong(vec3 position, vec3 normal, vec3 diffuse, vec3 glossy, float shininess, vec3 lightDir, vec3 lightIntensity) {
	vec3 wi = normalize(lightDir);
	vec3 w0 = normalize(-position);
	vec3 halfVector = (w0 + wi)/2.;
	return lightIntensity*(diffuse*(max(0., dot(wi,normal))) + glossy*(pow(max(0., dot(halfVector,normal)), shininess)));
}

void main() {

	vec3 position = texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 normal = texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 ambient = texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 diffuse = texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 glossy_shininess = texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0);
	vec3 glossy = glossy_shininess.xyz;
	float shininess = glossy_shininess.w;

	// Directional lights

	vec3 CumulatedDirectionalLight = vec3(0);
	
	for (int i = 0; i < uDirectionalLightNumber; i++) {
		vec3 DirectionalLightDir = directional_light_dirs[i].xyz;
		vec3 DirectionalLightIntensity = directional_light_intensities[i].xyz;
		CumulatedDirectionalLight += blinnPhong(position, normal, diffuse, glossy, shininess, DirectionalLightDir, DirectionalLightIntensity);
	}
	
	// Point lights
	
	vec3 CumulatedPointLight = vec3(0);
	
	for (int i = 0; i < uPointLightNumber; i++) {
		vec3 PointLightPos = point_light_positions[i].xyz;
		vec3 PointLightIntensity = point_light_intensities[i].xyz;
		float distToPointLight = length(PointLightPos - position);
		vec3 dirToPointLight = PointLightPos - position;
		vec3 intensityToPointLight = (PointLightIntensity) / (distToPointLight*distToPointLight);
		CumulatedPointLight += blinnPhong(position, normal, diffuse, glossy, shininess, dirToPointLight, intensityToPointLight);
	}
	
	fColor = CumulatedDirectionalLight + CumulatedPointLight + ambient;

}
