#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/load_obj.hpp>
#include <glmlv/ViewController.hpp>
#include "FreeflyCamera.hpp"
#include <glmlv/ViewController.hpp>
class Application
{
public:

	static const int MAX_LIGHT = 40;

	struct LightInfos { // vec4 for memory alignment
		glm::vec4 directional_light_dirs[MAX_LIGHT];
		glm::vec4 directional_light_intensities[MAX_LIGHT];
		glm::vec4 point_light_positions[MAX_LIGHT];
		glm::vec4 point_light_intensities[MAX_LIGHT];
	};

	struct DemoSceneObject {
		glmlv::ObjData objectData;
		GLuint VBO;
    	GLuint IBO;
    	GLuint VAO;
    	std::vector<GLuint> texObjects;
    	float scale;
    	glm::vec3 Position;
		float fPhi;
		float fTheta;
		float fPsi;
		glm::vec3 FrontVector;
		glm::vec3 LeftVector;
		glm::vec3 UpVector;
		FreeflyCamera camera;
	};
	
	enum DemoSceneObjectId {
		SceneObjectSponza = 0,
		SceneObjectTieFighter_1,
		SceneObjectTieFighter_2,
		SceneObjectTieInterceptor,
		SceneObjectTiePilot,
		SceneObjectInnerTie,
		SceneObjectArc170,
		SceneObjectInnerArc170,
		
		SceneObjectCount
	};
	
	enum PostEffect {
		PostEffectNone = 0,
		PostEffectGamma,
		PostEffectCount
	};

    Application(int argc, char** argv);
    
    ~Application();

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ int(m_nWindowWidth), int(m_nWindowHeight), "star wars demoscene" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;
    

    std::vector<DemoSceneObject> m_sceneObjects;
    int indexcam = 0;
    FreeflyCamera followCamera;
    FreeflyCamera innerArcCamera;
    FreeflyCamera innerTieCamera;
     std::vector<glm::vec3> DirectionalLightDirs = {
		glm::vec3(0,1,0)
	};
    std::vector<glm::vec3> DirectionalLightIntensities = {
		glm::vec3(0.4)
	};
    std::vector<glm::vec3> PointLightPositions = {
		glm::vec3(-1980,230,-65),
		glm::vec3(-2030,230,-65),
		glm::vec3(0,-630,-25)
	};
    std::vector<glm::vec3> PointLightIntensities = {
		glm::vec3(2000,0,0),
		glm::vec3(2000,0,0),
		glm::vec3(0,1000,0)
	};
    glm::mat4 currentViewMatrix;
    glmlv::ViewController viewController; //TODO rm;


    GLuint m_GVBO;
    GLuint m_GVAO;
    
    enum GBufferTextureType {
		GPosition = 0,
		GNormal,
		GAmbient,
		GDiffuse,
		GGlossyShininess,
		GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
		GBufferTextureCount
	};
	GLuint m_GBufferTextures[GBufferTextureCount];
	const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
    
    GLuint m_FBO;
    
	glm::vec3 m_bboxMin;
	glm::vec3 m_bboxMax;
    
    glmlv::ObjData::PhongMaterial m_default_material; // material without texture
    GLuint m_default_tex_object;

	GLuint m_samplerObject;

	GLuint m_directionalSMTexture;
	GLuint m_directionalSMFBO;
	GLuint m_directionalSMSampler;
	int32_t m_nDirectionalSMResolution = 4096;
	
	PostEffect currentEffect = PostEffectNone;
	GLuint m_BeautyTexture;
    GLuint m_BeautyFBO;
    GLuint m_GammaCorrectedBeautyTexture;
    GLuint m_GammaCorrectedBeautyFBO;

    
    glmlv::GLProgram m_programGeometryPass;
    glmlv::GLProgram m_programShadingPass;
    glmlv::GLProgram m_directionalSMProgram;
    glmlv::GLProgram m_gammaCorrectionProgram;

    GLint uModelViewProjMatrix;
    GLint uModelViewMatrix;
    GLint uNormalMatrix;
    GLint uDirectionalLightNumber;
	GLint uPointLightNumber;
	GLint uKa;
	GLint uKd;
	GLint uKs;
	GLint uShininess;
	GLint uKaSampler;
	GLint uKdSampler;
	GLint uKsSampler;
	GLint uShininessSampler;
	GLint uGPosition;
	GLint uGNormal;
	GLint uGAmbient;
	GLint uGDiffuse;
	GLint uGGlossyShininess;
	GLint uDirLightViewProjMatrix;
	GLint uShadowDirectionalLightDir;
	GLint uShadowDirectionalLightIntensity;
	GLint uSPDirLightViewProjMatrix;
	GLint uSPDirLightShadowMap;
	GLint uSPDirLightShadowMapBias;
	GLint uDirLightShadowMapSampleCount;
	GLint uDirLightShadowMapSpread;
	GLint uGammaExponent;
	
	GLuint ssboLightInfos;


	static DemoSceneObject createDemoSceneObject(const glmlv::fs::path & objPath);
	static DemoSceneObject createDemoSceneObject(const glmlv::fs::path & objPath, float scale, glm::vec3 translate, float phi, float theta);

	static void moveLeftDemoSceneObject(DemoSceneObject & sceneObject, float t);
	static void moveFrontDemoSceneObject(DemoSceneObject & sceneObject, float t);
	static void moveUpDemoSceneObject(DemoSceneObject & sceneObject, float t);
	static void rotateLeftDemoSceneObject(DemoSceneObject & sceneObject, float degrees);
	static void rotateUpDemoSceneObject(DemoSceneObject & sceneObject, float degrees);
	static void rotateFrontDemoSceneObject(DemoSceneObject & sceneObject, float degrees);
	static void computeDirectionVectorsDemoSceneObject(DemoSceneObject & sceneObject);
	glm::mat4 getTransformationMatrixDemoSceneObject(const DemoSceneObject & sceneObject);
	
	static void moveDemoSceneObject(DemoSceneObject & sceneObject, float speed, float left, float up, float front);
	
	static void deleteDemoSceneObject(DemoSceneObject & sceneObject);

	void moveFollowCamera(float speed, float left, float up, float front);

	void geometryPass(const glm::mat4 & ProjMatrix, const glm::mat4 & ViewMatrix);

	void shadingPass(
		const std::vector<glm::vec3> & DirectionalLightDirs, const std::vector<glm::vec3> & DirectionalLightIntensities,
		const std::vector<glm::vec3> & PointLightPositions, const std::vector<glm::vec3> & PointLightIntensities,
		const glm::vec3 & DirLightDirection, const glm::vec3 & DirectionalLightIntensity,
		const glm::mat4 & ViewMatrix, const glm::mat4 & rcpViewMatrix,
		const glm::mat4 & dirLightProjMatrix, const glm::mat4 & dirLightViewMatrix,
		float shadowMapBias, int shadowMapSampleCount, float shadowMapSpread
	);
	
	void computeShadowMap(glm::mat4 dirLightProjMatrix, glm::mat4 dirLightViewMatrix);
	
	void computePostEffect();
	
	unsigned long iterationSum(std::vector<unsigned long> current_iterations);
	float interpole(int t, int max, float fo, float fc);
	
	void loadSceneObjects();
	void animationArc170(unsigned long iteration);
	void animationTieFighter(unsigned long iteration);
	void animationFollowCamera(unsigned long iteration);
	void animationsetCamera(unsigned long iteration);
	void animationLights(unsigned long iteration);
	void animationEffect(unsigned long iteration);
	void animationSceneObjects(unsigned long iteration);

};
