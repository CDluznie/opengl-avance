#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/load_obj.hpp>
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
	};
	
	enum DemoSceneObjectId {
		SceneObjectSponza = 0,
		SceneObjectTieInterceptor_1,
		SceneObjectArc170,
		
		SceneObjectCount
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

    
    glmlv::GLProgram m_programGeometryPass;
    glmlv::GLProgram m_programShadingPass;
    glmlv::GLProgram m_directionalSMProgram;

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
	
	static void moveDemoSceneObject(DemoSceneObject & sceneObject, float speed, float left, float up);
	
	static void deleteDemoSceneObject(DemoSceneObject & sceneObject);

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
	
	unsigned long iterationSum(std::vector<unsigned long> current_iterations);
	
	void loadSceneObjects();
	void animationSceneObjects(double time, unsigned long iteration);

};
