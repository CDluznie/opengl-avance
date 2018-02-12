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

#include <algorithm> 


Application::DemoSceneObject Application::createDemoSceneObject(const glmlv::fs::path & objPath, glm::vec3 scale, glm::vec3 translate, glm::vec3 rotate) {
	Application::DemoSceneObject sceneObject;
	glmlv::loadObj(objPath, sceneObject.objectData);
	glGenBuffers(1, &sceneObject.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, sceneObject.VBO);
    glBufferStorage(GL_ARRAY_BUFFER, sceneObject.objectData.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), sceneObject.objectData.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &sceneObject.IBO);
    glBindBuffer(GL_ARRAY_BUFFER, sceneObject.IBO);
    glBufferStorage(GL_ARRAY_BUFFER, sceneObject.objectData.indexBuffer.size()*sizeof(uint32_t), sceneObject.objectData.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;
	glGenVertexArrays(1, &sceneObject.VAO);
    glBindVertexArray(sceneObject.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, sceneObject.VBO);
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sceneObject.IBO);
    glBindVertexArray(0);
    sceneObject.texObjects.resize(sceneObject.objectData.textures.size());
	glGenTextures(sceneObject.texObjects.size(), sceneObject.texObjects.data());
	for (int i = 0; i < sceneObject.texObjects.size(); i++){
		const auto texObject = sceneObject.texObjects[i];
		const glmlv::Image2DRGBA & image = sceneObject.objectData.textures[i];
		glBindTexture(GL_TEXTURE_2D, texObject);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	sceneObject.scale = scale;
	sceneObject.translate = translate;
	sceneObject.rotate = rotate;
	return sceneObject;
}

Application::DemoSceneObject Application::createDemoSceneObject(const glmlv::fs::path & objPath) {
	return createDemoSceneObject(objPath, glm::vec3(1.f), glm::vec3(0.f), glm::vec3(0.f));
}

void Application::deleteDemoSceneObject(Application::DemoSceneObject & sceneObject) {
	glDeleteBuffers(1, &sceneObject.VBO);
	glDeleteBuffers(1, &sceneObject.IBO);
    glDeleteBuffers(1, &sceneObject.VAO);
 	glDeleteTextures(sceneObject.texObjects.size(), sceneObject.texObjects.data());
}

void Application::geometryPass(const glm::mat4 & ProjMatrix, const glm::mat4 & ViewMatrix) {

	m_programGeometryPass.use();

  	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); 
        
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);

   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindSampler(0, m_samplerObject);
	glBindSampler(1, m_samplerObject);
	glBindSampler(2, m_samplerObject);
	glBindSampler(3, m_samplerObject);
	glBindSampler(4, m_samplerObject);

	// rendering code
		
	glUniform1i(uKaSampler, 0);
	glUniform1i(uKdSampler, 1);
	glUniform1i(uKsSampler, 2);
	glUniform1i(uShininessSampler, 3);
	
	for (const auto & sceneObject : m_sceneObjects) {
	
		glBindVertexArray(sceneObject.VAO);

		glm::mat4 MVMatrix = ViewMatrix;
		MVMatrix = glm::scale(MVMatrix, sceneObject.scale);//todo a l'init qu'une fois
		MVMatrix = glm::translate(MVMatrix, sceneObject.translate);
		if (sceneObject.rotate != glm::vec3(0.f)) { // bug quand rotate vaut 0,0,0
			MVMatrix = glm::rotate(MVMatrix, 1.f, sceneObject.rotate);
		}
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
	        
		glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
		glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	        
		auto indexOffset = 0;
		for (int i = 0; i < sceneObject.objectData.shapeCount; i++) {
			glmlv::ObjData::PhongMaterial material;
			const auto indexMaterial = sceneObject.objectData.materialIDPerShape[i];
			if (indexMaterial == -1) { // Use a default mat
				material = m_default_material;
			} else {
				material = sceneObject.objectData.materials[indexMaterial];
			}
			// Ambiant light
			GLuint KaTexture;
			if (material.KaTextureId == -1) {
				KaTexture = m_default_tex_object;
			} else {
				KaTexture = sceneObject.texObjects[material.KaTextureId];
			}
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, KaTexture);
			glUniform3fv(uKa, 1, glm::value_ptr(material.Ka));
			// Diffuse light
			GLuint KdTexture;
			if (material.KdTextureId == -1) {
				KdTexture = m_default_tex_object;
			} else {
				KdTexture = sceneObject.texObjects[material.KdTextureId];
			}
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, KdTexture);
			glUniform3fv(uKd, 1, glm::value_ptr(material.Kd));
			// Glossy light
			GLuint KsTexture;
			if (material.KsTextureId == -1) {
				KsTexture = m_default_tex_object;
			} else {
				KsTexture = sceneObject.texObjects[material.KsTextureId];
			}
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, KsTexture);
			glUniform3fv(uKs, 1, glm::value_ptr(material.Ks));
			// Glossy exponent
			GLuint ShininessTexture;
			if (material.shininessTextureId == -1) {
				ShininessTexture = m_default_tex_object;
			} else {
				ShininessTexture = sceneObject.texObjects[material.shininessTextureId];
			}
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ShininessTexture);
			glUniform1f(uShininess, material.shininess);
			// Render shape
			const auto indexCount = sceneObject.objectData.indexCountPerShape[i];
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;
		}

		glBindVertexArray(0);

	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

}

void Application::shadingPass(
	const std::vector<glm::vec3> & DirectionalLightDirs, const std::vector<glm::vec3> & DirectionalLightIntensities,
	const std::vector<glm::vec3> & PointLightPositions, const std::vector<glm::vec3> & PointLightIntensities,
	const glm::vec3 & DirLightDirection, const glm::vec3 & DirectionalLightIntensity,
	const glm::mat4 & ViewMatrix, const glm::mat4 & rcpViewMatrix,
	const glm::mat4 & dirLightProjMatrix, const glm::mat4 & dirLightViewMatrix,
	float shadowMapBias, int shadowMapSampleCount, float shadowMapSpread
) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_programShadingPass.use();


	glUniform1i(uGPosition, 0);
	glUniform1i(uGNormal, 1);
	glUniform1i(uGAmbient, 2);
	glUniform1i(uGDiffuse, 3);
	glUniform1i(uGGlossyShininess, 4);
        
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GNormal]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GAmbient]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDiffuse]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GGlossyShininess]);

	LightInfos lightInfos;
	for (int i = 0; i < DirectionalLightDirs.size(); i++) {
		lightInfos.directional_light_dirs[i] = ViewMatrix * glm::vec4(DirectionalLightDirs[i],0);
		lightInfos.directional_light_intensities[i] = glm::vec4(DirectionalLightIntensities[i],0);
	}
	for (int i = 0; i < PointLightPositions.size(); i++) {
		lightInfos.point_light_positions[i] = ViewMatrix * glm::vec4(PointLightPositions[i],1);
		lightInfos.point_light_intensities[i] = glm::vec4(PointLightIntensities[i],0);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLightInfos);
	glNamedBufferSubData(ssboLightInfos, 0, sizeof(LightInfos), &lightInfos); // todo
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glUniform1i(uDirectionalLightNumber, DirectionalLightDirs.size());
	glUniform1i(uPointLightNumber, PointLightPositions.size());

	const auto ShadowDirectionalLightDir = glm::vec3(ViewMatrix * glm::vec4(DirLightDirection,0));
	const auto ShadowDirectionalLightIntensity = DirectionalLightIntensity;
	glUniform3fv(uShadowDirectionalLightDir, 1, glm::value_ptr(ShadowDirectionalLightDir));
	glUniform3fv(uShadowDirectionalLightIntensity, 1, glm::value_ptr(ShadowDirectionalLightIntensity));

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
	glBindSampler(5, m_directionalSMSampler);

	glUniform1i(uSPDirLightShadowMap, 5);

	glUniformMatrix4fv(uSPDirLightViewProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix * rcpViewMatrix));
		
	glUniform1f(uSPDirLightShadowMapBias, shadowMapBias);
	glUniform1i(uDirLightShadowMapSampleCount, shadowMapSampleCount);
	glUniform1f(uDirLightShadowMapSpread, shadowMapSpread);

	glBindVertexArray(m_GVAO);

	glDrawArrays(GL_TRIANGLES, 0, 3);
        
	glBindVertexArray(0);

}

void Application::computeShadowMap(glm::mat4 dirLightProjMatrix, glm::mat4 dirLightViewMatrix) {

	m_directionalSMProgram.use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
	glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uDirLightViewProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix));
	
	for (const auto & sceneObject : m_sceneObjects) {

		glBindVertexArray(sceneObject.VAO);

		// We draw each shape by specifying how much indices it carries, and with an offset in the global index buffer
		auto indexOffset = 0;
		for (int i = 0; i < sceneObject.objectData.shapeCount; i++) {
			const auto indexCount = sceneObject.objectData.indexCountPerShape[i];
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;
		}

		glBindVertexArray(0);

	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
	
    std::vector<glm::vec3> DirectionalLightDirs = {};
    std::vector<glm::vec3> DirectionalLightIntensities = {};
    std::vector<glm::vec3> PointLightPositions = {};
    std::vector<glm::vec3> PointLightIntensities = {};

    float DirLightPhiAngleDegrees = 90;
	float DirLightThetaAngleDegrees = 90;
	glm::vec3 DirectionalLightIntensity(1.f);

	const auto sceneDiagonalSize = glm::length(m_bboxMax - m_bboxMin);

    glmlv::ViewController viewController(m_GLFWHandle.window());
    viewController.setSpeed(sceneDiagonalSize * 0.1f); // 10% de la scene parcouru par seconde
    
    float sceneSize = sceneDiagonalSize;
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.01f * sceneDiagonalSize, sceneSize);  // near = 1% de la taille de la scene, far = 100%
    
    bool directionalSMDirty = true;
    
    float shadowMapBias = 0.01f;
	int shadowMapSampleCount = 20;
	float shadowMapSpreadCoeff = 15.f;
	float shadowMapSpreadInit = 0.00001*0.3;
	float shadowMapSpread = shadowMapSpread*shadowMapSpreadCoeff;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        static const auto computeDirectionVectorUp = [](float phiRadians, float thetaRadians) {
		    const auto cosPhi = glm::cos(phiRadians);
		    const auto sinPhi = glm::sin(phiRadians);
		    const auto cosTheta = glm::cos(thetaRadians);
		    return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
		};

		const auto sceneCenter = 0.5f * (m_bboxMin + m_bboxMax);
		const float sceneRadius = sceneSize * 0.5f;

		float phi = glm::radians(DirLightPhiAngleDegrees);
		float theta = glm::radians(DirLightThetaAngleDegrees);
		const auto DirLightDirection = glm::vec3(glm::cos(phi) * glm::sin(theta), glm::sin(phi) * glm::sin(theta), glm::cos(theta));
		const auto dirLightUpVector = computeDirectionVectorUp(phi, theta);
		const auto dirLightViewMatrix = glm::lookAt(sceneCenter + DirLightDirection * sceneRadius, sceneCenter, dirLightUpVector); // Will not work if m_DirLightDirection is colinear to lightUpVector
		const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

		if (directionalSMDirty) {
			computeShadowMap(dirLightProjMatrix, dirLightViewMatrix);
			directionalSMDirty = false; // Pas de calcul au prochain tour
		}

		glm::mat4 ViewMatrix = viewController.getViewMatrix();

		geometryPass(ProjMatrix, ViewMatrix);

		shadingPass(
			DirectionalLightDirs, DirectionalLightIntensities,
			PointLightPositions, PointLightIntensities,
			DirLightDirection, DirectionalLightIntensity,
			ViewMatrix, viewController.getRcpViewMatrix(),
			dirLightProjMatrix, dirLightViewMatrix,
			shadowMapBias, shadowMapSampleCount, shadowMapSpread
		);
        

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
			if (ImGui::TreeNode("Shadow directional Light"))
			{
				float tmpDirectionalLightAngle[2] = {DirLightPhiAngleDegrees,DirLightThetaAngleDegrees};
				if(ImGui::InputFloat2("angle", tmpDirectionalLightAngle)) {
					DirLightPhiAngleDegrees = tmpDirectionalLightAngle[0];
					DirLightThetaAngleDegrees = tmpDirectionalLightAngle[1];
					directionalSMDirty = true;
				}
				ImGui::Separator();
				float tmpDirectionalLightIntensity[3] = {DirectionalLightIntensity.x, DirectionalLightIntensity.y, DirectionalLightIntensity.z};
				if(ImGui::InputFloat3("intensity", tmpDirectionalLightIntensity)) {
					DirectionalLightIntensity = glm::vec3(tmpDirectionalLightIntensity[0],tmpDirectionalLightIntensity[1], tmpDirectionalLightIntensity[2]);
					directionalSMDirty = true;
				}
				ImGui::Separator();
				ImGui::SliderFloat("Bias", &shadowMapBias, 0, 1, "%f");
				ImGui::Separator();
				ImGui::SliderInt("Sample count", &shadowMapSampleCount, 0, 100);
				ImGui::Separator();
				ImGui::SliderFloat("Spread", &shadowMapSpreadCoeff, 0, 100, "%f");
				shadowMapSpread = shadowMapSpreadInit*shadowMapSpreadCoeff;
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (ImGui::TreeNode("Directional Light"))
			{
				for (int i = 0; i < DirectionalLightDirs.size(); i++) {
					float tmpDirectionalLightDir[3] = {DirectionalLightDirs[i].x, DirectionalLightDirs[i].y, DirectionalLightDirs[i].z};
					float tmpDirectionalLightIntensity[3] = {DirectionalLightIntensities[i].x, DirectionalLightIntensities[i].y, DirectionalLightIntensities[i].z};
					std::string direction_label = "direction " + std::to_string(i+1);
					ImGui::InputFloat3(direction_label.c_str(), tmpDirectionalLightDir);
					std::string intensity_label = "intensity " + std::to_string(i+1);
					ImGui::InputFloat3(intensity_label.c_str(), tmpDirectionalLightIntensity);
					DirectionalLightDirs[i] = glm::vec3(tmpDirectionalLightDir[0],tmpDirectionalLightDir[1], tmpDirectionalLightDir[2]);
					DirectionalLightIntensities[i] = glm::vec3(tmpDirectionalLightIntensity[0],tmpDirectionalLightIntensity[1], tmpDirectionalLightIntensity[2]);
					ImGui::Separator();
				}
				if (ImGui::Button("Add light") && DirectionalLightDirs.size() < MAX_LIGHT) {
					DirectionalLightDirs.push_back(glm::vec3(0.f));
					DirectionalLightIntensities.push_back(glm::vec3(0.f));
				}
				if (ImGui::Button("Remove light") && DirectionalLightDirs.size() > 0) {
					DirectionalLightDirs.pop_back();
					DirectionalLightIntensities.pop_back();
				}
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (ImGui::TreeNode("Point Light"))
			{
				for (int i = 0; i < PointLightPositions.size(); i++) {
					float tmpPointLightPos[3] = {PointLightPositions[i].x, PointLightPositions[i].y, PointLightPositions[i].z};
					float tmpPointLightIntensity[3] = {PointLightIntensities[i].x, PointLightIntensities[i].y, PointLightIntensities[i].z};
					std::string direction_label = "position " + std::to_string(i+1);
					ImGui::InputFloat3(direction_label.c_str(), tmpPointLightPos);
					std::string intensity_label = "intensity " + std::to_string(i+1);
					ImGui::InputFloat3(intensity_label.c_str(), tmpPointLightIntensity);
					PointLightPositions[i] = glm::vec3(tmpPointLightPos[0],tmpPointLightPos[1], tmpPointLightPos[2]);
					PointLightIntensities[i] = glm::vec3(tmpPointLightIntensity[0], tmpPointLightIntensity[1], tmpPointLightIntensity[2]);
					ImGui::Separator();
				}
				if (ImGui::Button("Add light") && PointLightPositions.size() < MAX_LIGHT) {
					PointLightPositions.push_back(glm::vec3(0.f));
					PointLightIntensities.push_back(glm::vec3(0.f));
				}
				if (ImGui::Button("Remove light") && PointLightPositions.size() > 0) {
					PointLightPositions.pop_back();
					PointLightIntensities.pop_back();
				}
				ImGui::TreePop();
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

	m_sceneObjects.push_back(createDemoSceneObject(m_AssetsRootPath / m_AppName / "models" / "sponza" / "sponza.obj"));
	m_sceneObjects.push_back(createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "tie" / "imp_fly_tieinterceptor.obj",
		glm::vec3(20.f),glm::vec3(12.f,5.f,0.f),glm::vec3(0.f,-90.f,0.f)
	));

	m_default_material.Ka = glm::vec3(1.f);
	m_default_material.Kd = glm::vec3(1.f);
	m_default_material.Ks = glm::vec3(1.f);
	m_bboxMax = m_sceneObjects[0].objectData.bboxMax;
	m_bboxMin = m_sceneObjects[0].objectData.bboxMin;

	std::vector<glm::vec2> coveringTriangle = {
		glm::vec2(-1, -1),
		glm::vec2(3, -1),
		glm::vec2(-1, 3)
	};
	
	// --- VBO init ---


    glGenBuffers(1, &m_GVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_GVBO);
    glBufferStorage(GL_ARRAY_BUFFER, coveringTriangle.size()*sizeof(glm::vec2), coveringTriangle.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    
    // --- VAO init ----
    
	const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

    glGenVertexArrays(1, &m_GVAO);
    glBindVertexArray(m_GVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GVBO);
    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
  	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    
    // --- Program init ---
    
    m_programGeometryPass = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
    m_programShadingPass = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
    m_directionalSMProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "directionalSM.vs.glsl", m_ShadersRootPath / m_AppName / "directionalSM.fs.glsl" });

    uModelViewProjMatrix = glGetUniformLocation(m_programGeometryPass.glId(), "uModelViewProjMatrix");
    uModelViewMatrix = glGetUniformLocation(m_programGeometryPass.glId(), "uModelViewMatrix");
    uNormalMatrix = glGetUniformLocation(m_programGeometryPass.glId(), "uNormalMatrix");
	uKa = glGetUniformLocation(m_programGeometryPass.glId(), "uKa");
	uKd = glGetUniformLocation(m_programGeometryPass.glId(), "uKd");
	uKs = glGetUniformLocation(m_programGeometryPass.glId(), "uKs");
	uShininess = glGetUniformLocation(m_programGeometryPass.glId(), "uShininess");
	uKaSampler = glGetUniformLocation(m_programGeometryPass.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(m_programGeometryPass.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(m_programGeometryPass.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(m_programGeometryPass.glId(), "uShininessSampler");
	uDirectionalLightNumber = glGetUniformLocation(m_programShadingPass.glId(), "uDirectionalLightNumber");
	uPointLightNumber = glGetUniformLocation(m_programShadingPass.glId(), "uPointLightNumber");
	uGPosition = glGetUniformLocation(m_programShadingPass.glId(), "uGPosition");
	uGNormal = glGetUniformLocation(m_programShadingPass.glId(), "uGNormal");
	uGAmbient = glGetUniformLocation(m_programShadingPass.glId(), "uGAmbient");
	uGDiffuse = glGetUniformLocation(m_programShadingPass.glId(), "uGDiffuse");
	uGGlossyShininess = glGetUniformLocation(m_programShadingPass.glId(), "uGGlossyShininess");
	uDirLightViewProjMatrix = glGetUniformLocation(m_directionalSMProgram.glId(), "uDirLightViewProjMatrix");
	uShadowDirectionalLightDir = glGetUniformLocation(m_programShadingPass.glId(), "uShadowDirectionalLightDir");
	uShadowDirectionalLightIntensity = glGetUniformLocation(m_programShadingPass.glId(), "uShadowDirectionalLightIntensity");
	uSPDirLightViewProjMatrix = glGetUniformLocation(m_programShadingPass.glId(), "uSPDirLightViewProjMatrix");
	uSPDirLightShadowMap = glGetUniformLocation(m_programShadingPass.glId(), "uSPDirLightShadowMap");
	uSPDirLightShadowMapBias = glGetUniformLocation(m_programShadingPass.glId(), "uSPDirLightShadowMapBias");
	uDirLightShadowMapSampleCount = glGetUniformLocation(m_programShadingPass.glId(), "uDirLightShadowMapSampleCount");
	uDirLightShadowMapSpread = glGetUniformLocation(m_programShadingPass.glId(), "uDirLightShadowMapSpread");

	// --- Texture init ---
	
	glGenTextures(1, &m_default_tex_object);
	const glmlv::Image2DRGBA image {1, 1, 255, 255, 255, 255};
	glBindTexture(GL_TEXTURE_2D, m_default_tex_object);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenSamplers(1, &m_samplerObject);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_samplerObject, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //--- SSBO init for lights --- 
    
 	glGenBuffers(1, &ssboLightInfos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLightInfos); 
	glNamedBufferStorage(ssboLightInfos, sizeof(LightInfos), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboLightInfos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	// --- Framebuffer init ---
	
	glGenTextures(GBufferTextureCount, m_GBufferTextures);
	for (int i = 0; i < GBufferTextureCount; i++){
		const auto texObject = m_GBufferTextures[i];
		const auto texFormat = m_GBufferTextureFormat[i];
		glBindTexture(GL_TEXTURE_2D, texObject);
		glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, m_nWindowWidth, m_nWindowHeight);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); 

	for (int i = 0; i < GBufferTextureCount; i++){
		const auto attachment = (i == GDepth) ? (GL_DEPTH_ATTACHMENT) : (GL_COLOR_ATTACHMENT0 + i);
		const auto texObject = m_GBufferTextures[i];
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment,  GL_TEXTURE_2D, texObject, 0);
	}
	
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, drawBuffers);
	
	std::cerr << "Framebuffer status : " << (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) ==  GL_FRAMEBUFFER_COMPLETE) << std::endl;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

	// --- Shadow map init ---

	glGenTextures(1, &m_directionalSMTexture);
	glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
	glBindTexture(GL_TEXTURE_2D, 0);
    
	glGenFramebuffers(1, &m_directionalSMFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);
	std::cerr << "Shadow map Framebuffer status : " << (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) ==  GL_FRAMEBUFFER_COMPLETE) << std::endl;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenSamplers(1, &m_directionalSMSampler);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

}

Application::~Application() {
    glDeleteBuffers(1, &m_GVBO);
    glDeleteBuffers(1, &m_GVAO);
    glDeleteBuffers(1, &ssboLightInfos);
    glDeleteTextures(1, &m_default_tex_object);
    glDeleteTextures(GBufferTextureCount, m_GBufferTextures);
    glDeleteSamplers(1, &m_samplerObject);
    std::for_each(m_sceneObjects.begin(), m_sceneObjects.end(), deleteDemoSceneObject);
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}
