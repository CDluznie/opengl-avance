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
uniform sampler2D uGDepth;
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

int kernelSize = 14;
uniform vec3 uKernels[14];
uniform vec3 uNoiseVec;
uniform mat4 uProjectionMat;
uniform sampler2D uTexNoise;	

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

	float uRadius = 10;
	float bias = 0.15;
	
	
	// SSAO
	
	/*
	float depth = texelFetch(uGDepth, ivec2(gl_FragCoord.xy), 0).x;

	
	float www = 1280;
	float hhh = 720;
    vec2 TexCoords = gl_FragCoord.xy * vec2(1/www,1/hhh);
	vec2 noiseScale = vec2(www/4.0, hhh/4.0);
	vec3 randomVec = texture(uTexNoise, TexCoords * noiseScale).xyz;  
	
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);  
	
	float occlusion = 0.0;
	
	vec2 texelSize = 1.0 / vec2(www,hhh);
    float result = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            //result += texture(ssaoInput, TexCoords + offset).r;
            float occlusion = 0;
            for(int i = 0; i < kernelSize; ++i) {
				
				vec3 sampleRay = tbn * uKernels[i] * uRadius + position;
				
				vec4 offsetScreen = uProjectionMat * vec4(sampleRay.xyz, 1);
				vec3 offsetNDC = vec3(offsetScreen / offsetScreen.w) * 0.5 + 0.5;
				
				
				float sampleDepth = texture(uGPosition, offsetNDC.xy).z;
				
				float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(position.z - sampleDepth));
				occlusion += (sampleDepth >= sampleRay.z + bias ? 1.0 : 0.0) * rangeCheck;  
		   
			}
			occlusion = 1.0 - (occlusion/kernelSize);
			result += occlusion;
        }
    }
    occlusion = result / (4.0 * 4.0);
	*/
	

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
	
	 
	//fColor = occlusion*(CumulatedDirectionalLight + CumulatedPointLight) + 0.25*ambient;
	fColor = (CumulatedDirectionalLight + CumulatedPointLight) + 0.25*ambient;
		
	
}



