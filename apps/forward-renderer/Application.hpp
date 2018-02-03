#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/load_obj.hpp>

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

    Application(int argc, char** argv);
    
    ~Application();

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ int(m_nWindowWidth), int(m_nWindowHeight), "forward-renderer" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;
    
    GLuint m_VBO;
    GLuint m_IBO;
    GLuint m_VAO;
    
	glm::vec3 m_bboxMin;
	glm::vec3 m_bboxMax;
    
    size_t m_shapeCount;
    std::vector<uint32_t> m_indexCountPerShape;
    std::vector<int32_t> m_materialIDPerShape;
    
    glmlv::ObjData::PhongMaterial m_default_material; // material without texture
    std::vector<glmlv::ObjData::PhongMaterial> m_materials;
    GLuint m_default_tex_object;
    std::vector<GLuint> m_texObjects;
	GLuint m_samplerObject;
    
    glmlv::GLProgram m_program;
    
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
	
	GLuint ssboLightInfos;
	
};
