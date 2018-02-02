#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/load_obj.hpp>
#include <glmlv/ViewController.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    
    glm::vec3 DirectionalLightDir(0.f,0.f,1.f);
    glm::vec3 DirectionalLightIntensity(0.5f);
	glm::vec3 PointLightPosition(0.f, 200.f, 0.f);
	glm::vec3 PointLightIntensity(100000.f);
	
	const auto sceneDiagonalSize = glm::length(m_bboxMax - m_bboxMin);

    glmlv::ViewController viewController(m_GLFWHandle.window());
    viewController.setSpeed(sceneDiagonalSize * 0.1f); // 10% de la scene parcouru par seconde
    
    float m_SceneSize = 2000.f;
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.01f * sceneDiagonalSize, sceneDiagonalSize);  // near = 1% de la taille de la scene, far = 100%
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rendering code
        
		glm::mat4 ViewMatrix = viewController.getViewMatrix();
		glm::mat4 MVMatrix = glm::translate(ViewMatrix, glm::vec3(0.f, 0.f, -5.f));
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
  
        glm::vec3 lightDir = glm::vec3(ViewMatrix * glm::vec4(DirectionalLightDir,0));
        glm::vec3 lightPos = glm::vec3(ViewMatrix * glm::vec4(PointLightPosition,1));
        
        glUniform3fv(uDirectionalLightDir, 1, glm::value_ptr(lightDir));
		glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(DirectionalLightIntensity));
		glUniform3fv(uPointLightPosition, 1, glm::value_ptr(lightPos));
		glUniform3fv(uPointLightIntensity, 1, glm::value_ptr(PointLightIntensity));
		
        glUniform1i(uKaSampler, 0);
        glUniform1i(uKdSampler, 1);
        glUniform1i(uKsSampler, 2);
        glUniform1i(uShininessSampler, 3);
		
        glBindVertexArray(m_VAO);
        
		glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
		glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        
        auto indexOffset = 0;
		for (int i = 0; i < m_shapeCount; i++) {
			glmlv::ObjData::PhongMaterial material;
			const auto indexMaterial = m_materialIDPerShape[i];
			if (indexMaterial == -1) { // Use a default mat
				material = m_default_material;
			} else {
				material = m_materials[indexMaterial];
			}
			// Ambiant light
			GLuint KaTexture;
			if (material.KaTextureId == -1) {
				KaTexture = m_default_tex_object;
			} else {
				KaTexture = m_texObjects[material.KaTextureId];
			}
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, KaTexture);
			glUniform3fv(uKa, 1, glm::value_ptr(material.Ka));
			// Diffuse light
			GLuint KdTexture;
			if (material.KdTextureId == -1) {
				KdTexture = m_default_tex_object;
			} else {
				KdTexture = m_texObjects[material.KdTextureId];
			}
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, KdTexture);
			glUniform3fv(uKd, 1, glm::value_ptr(material.Kd));
			// Glossy light
			GLuint KsTexture;
			if (material.KsTextureId == -1) {
				KsTexture = m_default_tex_object;
			} else {
				KsTexture = m_texObjects[material.KsTextureId];
			}
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, KsTexture);
			glUniform3fv(uKs, 1, glm::value_ptr(material.Ks));
			// Glossy exponent
			GLuint ShininessTexture;
			if (material.shininessTextureId == -1) {
				ShininessTexture = m_default_tex_object;
			} else {
				ShininessTexture = m_texObjects[material.shininessTextureId];
			}
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ShininessTexture);
			glUniform1f(uShininess, material.shininess);
			// Render shape
			const auto indexCount = m_indexCountPerShape[i];
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;
		}
        
        glBindVertexArray(0);
        
        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();
        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
			ImGui::Separator();
			if (ImGui::TreeNode("Directional Light"))
			{
				float tmpDirectionalLightDir[3] = {DirectionalLightDir.x, DirectionalLightDir.y, DirectionalLightDir.z};
				float tmpDirectionalLightIntensity[3] = {DirectionalLightIntensity.x, DirectionalLightIntensity.y, DirectionalLightIntensity.z};
				ImGui::InputFloat3("direction", tmpDirectionalLightDir);
				ImGui::InputFloat3("intensity", tmpDirectionalLightIntensity);
				ImGui::TreePop();
				DirectionalLightDir = glm::vec3(tmpDirectionalLightDir[0],tmpDirectionalLightDir[1], tmpDirectionalLightDir[2]);
				DirectionalLightIntensity = glm::vec3(tmpDirectionalLightIntensity[0],tmpDirectionalLightIntensity[1], tmpDirectionalLightIntensity[2]);
			}
			ImGui::Separator();
			if (ImGui::TreeNode("Point Light"))
			{
				float tmpPointLightPosition[3] = {PointLightPosition.x, PointLightPosition.y, PointLightPosition.z};
				float tmpPointLightIntensity[3] = {PointLightIntensity.x, PointLightIntensity.y, PointLightIntensity.z};
				ImGui::InputFloat3("position", tmpPointLightPosition);
				ImGui::InputFloat3("intensity", tmpPointLightIntensity);
				ImGui::TreePop();
				PointLightPosition = glm::vec3(tmpPointLightPosition[0],tmpPointLightPosition[1], tmpPointLightPosition[2]);
				PointLightIntensity = glm::vec3(tmpPointLightIntensity[0],tmpPointLightIntensity[1], tmpPointLightIntensity[2]);
			}
			ImGui::End();
        }

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
	m_AssetsRootPath{ m_AppPath.parent_path() / "assets" }  
  
{	
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	glEnable(GL_DEPTH_TEST);

	glmlv::ObjData objectData;
	glmlv::loadObj(m_AssetsRootPath / m_AppName / "models" / "sponza" / "sponza.obj", objectData);
	
	m_indexCountPerShape = objectData.indexCountPerShape;
	m_shapeCount = objectData.shapeCount;
	m_materialIDPerShape = objectData.materialIDPerShape;
	m_materials = objectData.materials;
	m_default_material.Ka = glm::vec3(1.f);
	m_default_material.Kd = glm::vec3(1.f);
	m_default_material.Ks = glm::vec3(1.f);
	m_bboxMax = objectData.bboxMax;
	m_bboxMin = objectData.bboxMin;
	
	// --- VBO init ---

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferStorage(GL_ARRAY_BUFFER, objectData.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), objectData.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    // --- IBO init ---
    
    glGenBuffers(1, &m_IBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
    glBufferStorage(GL_ARRAY_BUFFER, objectData.indexBuffer.size()*sizeof(uint32_t), objectData.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
    // --- VAO init ----
    
	const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;
    
	glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBindVertexArray(0);
    
    
    // --- Program init ---
    
    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });

    uModelViewProjMatrix = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    uModelViewMatrix = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    uNormalMatrix = glGetUniformLocation(m_program.glId(), "uNormalMatrix");
    uDirectionalLightDir = glGetUniformLocation(m_program.glId(), "uDirectionalLightDir");
    uDirectionalLightIntensity = glGetUniformLocation(m_program.glId(), "uDirectionalLightIntensity");
	uPointLightPosition = glGetUniformLocation(m_program.glId(), "uPointLightPosition");
	uPointLightIntensity = glGetUniformLocation(m_program.glId(), "uPointLightIntensity");
	uKa = glGetUniformLocation(m_program.glId(), "uKa");
	uKd = glGetUniformLocation(m_program.glId(), "uKd");
	uKs = glGetUniformLocation(m_program.glId(), "uKs");
	uShininess = glGetUniformLocation(m_program.glId(), "uShininess");
	uKaSampler = glGetUniformLocation(m_program.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(m_program.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(m_program.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(m_program.glId(), "uShininessSampler");
	
	// --- Texture init ---

	m_texObjects.resize(objectData.textures.size());

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(m_texObjects.size(), m_texObjects.data());
	for (int i = 0; i < m_texObjects.size(); i++){
		const auto texObject = m_texObjects[i];
		const glmlv::Image2DRGBA & image = objectData.textures[i];
		glBindTexture(GL_TEXTURE_2D, texObject);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glGenTextures(1, &m_default_tex_object);
	const glmlv::Image2DRGBA image {1, 1, 255, 255, 255, 255};
	glBindTexture(GL_TEXTURE_2D, m_default_tex_object);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenSamplers(1, &m_samplerObject);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    glBindSampler(0, m_samplerObject);
	glBindSampler(1, m_samplerObject);
	glBindSampler(2, m_samplerObject);
	glBindSampler(3, m_samplerObject);
	
    m_program.use();
    
}

Application::~Application() {
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_IBO);
    glDeleteBuffers(1, &m_VAO);
    glDeleteTextures(m_texObjects.size(), m_texObjects.data());
    glDeleteTextures(m_default_tex_object, &m_default_tex_object);
    glDeleteSamplers(1, &m_samplerObject);
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}
