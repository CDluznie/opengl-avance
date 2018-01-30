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
	glm::vec3 PointLightPosition(2.5f, 0.f, -4.f);
	glm::vec3 PointLightIntensity(2.5f);
	glm::vec3 Kd(1.f);

    glmlv::ViewController viewController(m_GLFWHandle.window());
    
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.1f, 100.f);
    
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
		glUniform3fv(uKd, 1, glm::value_ptr(Kd));
        glUniform1i(uKdSampler, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, m_samplerObject);
        
        glBindVertexArray(m_VAO);
        
		glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
		glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        
        auto indexOffset = 0;
		for (int i = 0; i < m_shapeCount; i++) {
			const auto indexCount = m_indexCountPerShape[i];
			//TODO get i-th materialIDPerShape
			//glBindTexture(GL_TEXTURE_2D, m_texObject);
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
			ImGui::Separator();
			if (ImGui::TreeNode("Light Color"))
			{
				float tmpKd[3] = {Kd.x, Kd.y, Kd.z};
				ImGui::InputFloat3("color", tmpKd);
				ImGui::TreePop();
				Kd = glm::vec3(tmpKd[0],tmpKd[1], tmpKd[2]);
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
	//glmlv::loadObj(m_AssetsRootPath / m_AppName / "models" / "sponza" / "sponza.obj", objectData);
	glmlv::loadObj(m_AssetsRootPath / m_AppName / "models" / "stage" / "stage.obj", objectData);
	
	m_indexCountPerShape = objectData.indexCountPerShape;
	m_shapeCount = objectData.shapeCount;
	
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
	uKd = glGetUniformLocation(m_program.glId(), "uKd");
	uKdSampler = glGetUniformLocation(m_program.glId(), "uKdSampler");
	
	// --- Texture init ---
	
	// TODO load all textures
	/*const glmlv::Image2DRGBA & image = objectData.textures.front();

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &m_texObject); // TODO tex array
	glBindTexture(GL_TEXTURE_2D, m_texObject);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	*/
	
	glGenSamplers(1, &m_samplerObject);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    m_program.use();
    
}

Application::~Application() {
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_IBO);
    glDeleteBuffers(1, &m_VAO);
    glDeleteTextures(1, &m_texObject);
    glDeleteSamplers(1, &m_samplerObject);
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}
