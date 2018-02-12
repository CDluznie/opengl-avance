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

uniform vec3 uShadowDirectionalLightDir;
uniform vec3 uShadowDirectionalLightIntensity;

uniform mat4 uSPDirLightViewProjMatrix;
uniform sampler2DShadow uSPDirLightShadowMap;
uniform float uSPDirLightShadowMapBias;

uniform int uDirLightShadowMapSampleCount;
uniform float uDirLightShadowMapSpread;

vec2 poissonDisk[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);

float random(vec4 seed) {
    float dot_product = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

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

	vec4 positionInDirLightScreen = uSPDirLightViewProjMatrix * vec4(position, 1);
	vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5;

	float dirLightVisibility = 0.0;
	float dirSampleCountf = float(uDirLightShadowMapSampleCount);
	int step = max(1, 16 / uDirLightShadowMapSampleCount);
	for (int i = 0; i < uDirLightShadowMapSampleCount; ++i) {
	    //int index = int(dirSampleCountf * random(vec4(gl_FragCoord.xyy, i))) % uDirLightShadowMapSampleCount;
    	int index = (i + step) % uDirLightShadowMapSampleCount;
    	dirLightVisibility += textureProj(uSPDirLightShadowMap, vec4(positionInDirLightNDC.xy + uDirLightShadowMapSpread * poissonDisk[index], positionInDirLightNDC.z - uSPDirLightShadowMapBias, 1.0), 0.0);
	}
	dirLightVisibility /= dirSampleCountf;

	// Directional lights

	vec3 CumulatedDirectionalLight = vec3(0);
	
	for (int i = 0; i < uDirectionalLightNumber; i++) {
		vec3 DirectionalLightDir = directional_light_dirs[i].xyz;
		vec3 DirectionalLightIntensity = directional_light_intensities[i].xyz;
		CumulatedDirectionalLight += blinnPhong(position, normal, diffuse, glossy, shininess, DirectionalLightDir, DirectionalLightIntensity);
	}
	
	CumulatedDirectionalLight +=  dirLightVisibility *  blinnPhong(position, normal, diffuse, glossy, shininess, uShadowDirectionalLightDir, uShadowDirectionalLightIntensity);

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
	
	fColor = CumulatedDirectionalLight + CumulatedPointLight + 0.25*ambient;

}
