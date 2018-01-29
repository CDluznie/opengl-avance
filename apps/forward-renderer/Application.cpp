#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/ViewController.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    
    glmlv::ViewController viewController(m_GLFWHandle.window());
    
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.1f, 100.f);
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rendering code
        
		glm::mat4 GlobalMVMatrix = glm::translate(viewController.getViewMatrix(), glm::vec3(0.f, 0.f, -5.f));
		glm::mat4 CubeMVMatrix = glm::scale(GlobalMVMatrix, glm::vec3(1.5f));
		glm::mat4 CubeNormalMatrix = glm::transpose(glm::inverse(CubeMVMatrix));
		glm::mat4 SphereMVMatrix = glm::translate(GlobalMVMatrix, glm::vec3(-2.f, 0.5f, -0.5f));
		glm::mat4 SphereNormalMatrix = glm::transpose(glm::inverse(SphereMVMatrix));
        
        glBindVertexArray(m_cubeVAO);
        
		glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * CubeMVMatrix));
		glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(CubeMVMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(CubeNormalMatrix));
        
        glDrawElements(GL_TRIANGLES, m_cube_vertex_number, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);
        
        glBindVertexArray(m_sphereVAO);
        
		glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * SphereMVMatrix));
		glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(SphereMVMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(SphereNormalMatrix));
        
        glDrawElements(GL_TRIANGLES, m_sphere_vertex_number, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);
        
        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
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
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }
    
{	
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	glEnable(GL_DEPTH_TEST);

	glmlv::SimpleGeometry cube = glmlv::makeCube();
	m_cube_vertex_number = cube.indexBuffer.size();
	glmlv::SimpleGeometry sphere = glmlv::makeSphere(20);
	m_sphere_vertex_number = sphere.indexBuffer.size();

	// --- VBO init ---

	glGenBuffers(1, &m_cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferStorage(GL_ARRAY_BUFFER, cube.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), cube.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &m_sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
    glBufferStorage(GL_ARRAY_BUFFER, sphere.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), sphere.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    // --- IBO init ---
    
    glGenBuffers(1, &m_cubeIBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeIBO);
    glBufferStorage(GL_ARRAY_BUFFER, cube.indexBuffer.size()*sizeof(uint32_t), cube.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &m_sphereIBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereIBO);
    glBufferStorage(GL_ARRAY_BUFFER, sphere.indexBuffer.size()*sizeof(uint32_t), sphere.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // --- VAO init ----
    
	const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;
    
	glGenVertexArrays(1, &m_cubeVAO);
    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIBO);
    glBindVertexArray(0);
    
	glGenVertexArrays(1, &m_sphereVAO);
    glBindVertexArray(m_sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphereIBO);
    glBindVertexArray(0);
    
    // --- Program init ---
    
    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });

    uModelViewProjMatrix = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    uModelViewMatrix = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    uNormalMatrix = glGetUniformLocation(m_program.glId(), "uNormalMatrix");
    
    m_program.use();
    
}

Application::~Application() {
	glDeleteBuffers(1, &m_cubeVBO);
	glDeleteBuffers(1, &m_sphereVBO);
	glDeleteBuffers(1, &m_cubeIBO);
	glDeleteBuffers(1, &m_sphereIBO);
    glDeleteBuffers(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_sphereVAO);
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}
