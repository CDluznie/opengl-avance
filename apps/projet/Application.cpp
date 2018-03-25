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

#include "FreeflyCamera.hpp"

Application::DemoSceneObject Application::createDemoSceneObject(const glmlv::fs::path & objPath, float scale, glm::vec3 translate, float phi, float theta) {
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
	sceneObject.Position = translate/scale;
	sceneObject.fPhi = glm::radians(phi);
	sceneObject.fTheta = glm::radians(theta);
	sceneObject.fPsi = 0;
	computeDirectionVectorsDemoSceneObject(sceneObject);
	sceneObject.camera = FreeflyCamera(translate, phi, theta);
	sceneObject.outCamera = FreeflyCamera(translate, phi, theta);
	return sceneObject;
}

void Application::resetDemoSceneObject(DemoSceneObject & sceneObject, glm::vec3 translate, float phi, float theta) {
	sceneObject.Position = translate/sceneObject.scale;
	sceneObject.fPhi = glm::radians(phi);
	sceneObject.fTheta = glm::radians(theta);
	sceneObject.fPsi = 0;
	computeDirectionVectorsDemoSceneObject(sceneObject);
	sceneObject.camera = FreeflyCamera(translate, phi, theta);
	sceneObject.outCamera = FreeflyCamera(translate, phi, theta);
}

Application::DemoSceneObject Application::createDemoSceneObject(const glmlv::fs::path & objPath) {
	return createDemoSceneObject(objPath, 1.f, glm::vec3(0.f), 180, 0);
}

void Application::moveLeftDemoSceneObject(DemoSceneObject & sceneObject, float t) {
	sceneObject.Position += (t/sceneObject.scale)*sceneObject.LeftVector;
}

void Application::moveFrontDemoSceneObject(DemoSceneObject & sceneObject, float t) {
	sceneObject.Position += (t/sceneObject.scale)*sceneObject.FrontVector;
}

void Application::moveUpDemoSceneObject(DemoSceneObject & sceneObject, float t) {
	sceneObject.Position += (t/sceneObject.scale)*sceneObject.UpVector;
}

void Application::rotateLeftDemoSceneObject(DemoSceneObject & sceneObject, float degrees) {
	sceneObject.fPhi += glm::radians(degrees);
	computeDirectionVectorsDemoSceneObject(sceneObject);
}

void Application::rotateUpDemoSceneObject(DemoSceneObject & sceneObject, float degrees) {
	sceneObject.fTheta += glm::radians(degrees);
	computeDirectionVectorsDemoSceneObject(sceneObject);
}

void Application::rotateFrontDemoSceneObject(DemoSceneObject & sceneObject, float degrees) {
	sceneObject.fPsi += glm::radians(degrees);
	computeDirectionVectorsDemoSceneObject(sceneObject);
}

void Application::computeDirectionVectorsDemoSceneObject(DemoSceneObject & sceneObject) {
	double cosTheta = glm::cos(sceneObject.fTheta), sinTheta = glm::sin(sceneObject.fTheta);
	double cosPhi = glm::cos(sceneObject.fPhi), sinPhi = glm::sin(sceneObject.fPhi);
	sceneObject.FrontVector = glm::vec3(cosTheta*sinPhi, sinTheta, cosTheta*cosPhi);
	sceneObject.LeftVector = glm::vec3(cosPhi, 0, -sinPhi);
	//sceneObject.LeftVector = glm::vec3(glm::sin(sceneObject.fPhi + glm::half_pi<float>()), 0, glm::cos(sceneObject.fPhi + glm::half_pi<float>()));
	sceneObject.UpVector = glm::cross(sceneObject.FrontVector, sceneObject.LeftVector);
	
}

void Application::moveDemoSceneObject(DemoSceneObject & sceneObject, float speed, float left, float up, float front) {
	moveFrontDemoSceneObject(sceneObject, speed);
	rotateUpDemoSceneObject(sceneObject, up);
	rotateLeftDemoSceneObject(sceneObject, left);
	rotateFrontDemoSceneObject(sceneObject, front);
	sceneObject.camera.setPosition(sceneObject.Position * sceneObject.scale);
	sceneObject.camera.setHorizontalAngle(sceneObject.fPhi);
	sceneObject.camera.setVerticalAngle(sceneObject.fTheta);
	sceneObject.camera.setFrontAngle(sceneObject.fPsi);
	
	
	glm::mat4 psiRotation = glm::rotate(glm::mat4(1.f), (sceneObject.fPsi), sceneObject.FrontVector);
	glm::vec3 lv = glm::vec3(glm::vec4(sceneObject.LeftVector,0) * psiRotation);
	glm::vec3 uv = glm::vec3(glm::vec4(sceneObject.UpVector,0) * psiRotation);
	glm::vec3 fv = glm::vec3(glm::vec4(sceneObject.FrontVector,0) * psiRotation);
	glm::vec3 pos = sceneObject.Position * sceneObject.scale 
		+ 27.5f*lv
		+ 20.f*uv
		- 10.f*fv
	;
	glm::vec3 targetpos = pos - lv;
	sceneObject.OutCamera = glm::lookAt(pos, targetpos, uv);
}

glm::mat4 Application::getTransformationMatrixDemoSceneObject(const DemoSceneObject & sceneObject) {
	glm::mat4 transformations(1.f);
	transformations = glm::scale(transformations, glm::vec3(sceneObject.scale));
	transformations = glm::translate(transformations, sceneObject.Position);
	transformations = glm::rotate(transformations, sceneObject.fPhi, glm::vec3(0,1,0));
	transformations = glm::rotate(transformations, sceneObject.fTheta, glm::vec3(-1,0,0));
	transformations = glm::rotate(transformations, sceneObject.fPsi, glm::vec3(0,0,-1));
	return transformations;
}

void Application::deleteDemoSceneObject(Application::DemoSceneObject & sceneObject) {
	glDeleteBuffers(1, &sceneObject.VBO);
	glDeleteBuffers(1, &sceneObject.IBO);
    glDeleteBuffers(1, &sceneObject.VAO);
 	glDeleteTextures(sceneObject.texObjects.size(), sceneObject.texObjects.data());
}

void Application::moveFollowCamera(float speed, float left, float up, float front) {
	followCamera.moveFront(speed);
	followCamera.rotateHorizontalAngle(left);
	followCamera.rotateVerticalAngle(up);
	followCamera.rotateFrontAngle(front);
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

		glm::mat4 MVMatrix = ViewMatrix* getTransformationMatrixDemoSceneObject(sceneObject);
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
	        
	        
	    toto = ProjMatrix;    
	        
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

float RandomFloat(float min, float max) {
    return  (max - min) * ((((float) rand()) / (float) RAND_MAX)) + min ;
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
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BeautyFBO); 


	glUniform1i(uGPosition, 0);
	glUniform1i(uGNormal, 1);
	glUniform1i(uGAmbient, 2);
	glUniform1i(uGDiffuse, 3);
	glUniform1i(uGGlossyShininess, 4);
	glUniform1i(uGDepth, 6);
	
	glUniform1i(uGDepth, 7);
        
        
    glUniformMatrix4fv(uProjMat, 1, GL_FALSE, glm::value_ptr(toto));    
    
    glUniform3fv(uKernels, kernelSize, (kernels.data()));
 
 
	glBindSampler(7, samplerNoise);
 
    glUniform3fv(uNoiseVec, 1, glm::value_ptr(noise[0])); //TODO use only 0
        
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GNormal]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GAmbient]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDiffuse]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDepth]);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

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
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

}

void Application::computeShadowMap(glm::mat4 dirLightProjMatrix, glm::mat4 dirLightViewMatrix) {

	m_directionalSMProgram.use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
	glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const auto & sceneObject : m_sceneObjects) {

		glUniformMatrix4fv(uDirLightViewProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix * getTransformationMatrixDemoSceneObject(sceneObject)));

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

void Application::computePostEffect() {
	if (currentEffect == PostEffectNone) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_BeautyFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(
        	0,0,m_nWindowWidth,m_nWindowHeight,
        	0,0,m_nWindowWidth,m_nWindowHeight,
        	GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        return;
	}
	if (currentEffect == PostEffectGamma) {
		m_gammaCorrectionProgram.use();
		glUniform1f(uGammaExponent, 0.7);
	} else if (currentEffect == PostEffectBlackFade) {
		m_blackFadeProgram.use();
		glUniform1f(uAlpha, blackFadeAlpha);
	} else {
		return;
	}
	glBindImageTexture(0, m_BeautyTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_GammaCorrectedBeautyTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(m_nWindowWidth, m_nWindowHeight, 1);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GammaCorrectedBeautyFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(
		0,0,m_nWindowWidth,m_nWindowHeight,
		0,0,m_nWindowWidth,m_nWindowHeight,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

unsigned long Application::iterationSum(std::vector<unsigned long> current_iterations) {
	unsigned long sum = 0;
	for (auto& n : current_iterations)
		sum += n;
	return sum;
}

void Application::loadSceneObjects() {
	m_sceneObjects.resize(SceneObjectCount);
	m_sceneObjects[SceneObjectSponza] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "sponza" / "sponza.obj"
	);
	m_sceneObjects[SceneObjectTieFighter_1] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "tief" / "imp_fly_tiefighter.obj",
		5.f,glm::vec3(-1200.f, 500.f, -400.f),-90,0
	);
	m_sceneObjects[SceneObjectArc170] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "arc" / "Arc170.obj",
		0.05f, glm::vec3(-1200.f,500.f,480.f),90,0
	);
	m_sceneObjects[SceneObjectInnerArc170] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "arc" / "Arc170.obj",
		1.5f, glm::vec3(0,-1000,0),0,0
	);
	innerArcCamera = FreeflyCamera(
		glm::vec3(0,-635,-17),
		180,0
	);
	m_sceneObjects[SceneObjectInnerTie] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "tiei" / "imp_fly_tieinterceptor.obj",
		51.75f,glm::vec3(0.f, -2900.f,0.f),0,0
	);
	m_sceneObjects[SceneObjectTiePilot] = createDemoSceneObject(
		m_AssetsRootPath / m_AppName / "models" / "tiep" / "imp_inf_pilot.obj",
		115.f, glm::vec3(0,-3040,0),0,0
	);
	innerTieCamera = FreeflyCamera(
		glm::vec3(0,-2830,-65),
		0,0
	);
	followCamera = FreeflyCamera(
		glm::vec3(-1300.f,525.f,480.f),
		90,0
	);
}

void Application::resetSceneObjects() {
	resetDemoSceneObject(
		m_sceneObjects[SceneObjectTieFighter_1],
		glm::vec3(-1200.f, 500.f, -400.f),-90,0
	);
	resetDemoSceneObject(
		m_sceneObjects[SceneObjectArc170],
		glm::vec3(-1200.f,500.f,480.f),90,0
	);
	followCamera = FreeflyCamera(
		glm::vec3(-1300.f,525.f,480.f),
		90,0
	);
}

void Application::animationArc170(unsigned long iteration) {
	std::vector<unsigned long> times;
	float speed;
	speed = 0.6;
	times.push_back(2500);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 1;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 1.5;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 2;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.15, 0, 0.2);
		return;
	}
	times.push_back(200);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.3, 0, 0.2);
		return;
	};
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.1, 0, -0.2);
		return;
	}
	times.push_back(200);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.3, 0, -0.2);
		return;
	}
	times.push_back(200);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.1, 0., -0.2);
		return;
	}
	times.push_back(350);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.092, 0.03, -0.2);
		return;
	}
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, -0.06, -0.2, 0.2);
		return;
	}
	times.push_back(200);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, -0.06, 0.14, 0.1);
		return;
	}
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, -0.35, 0.14, -0.1);
		return;
	}
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, -0.35, -0.02, -0.1);
		return;
	}
	times.push_back(800);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, -0.02, 0.05);
		return;
	}
	times.push_back(500);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, -0.38, 0.01, 0);
		return;
	}
	times.push_back(700);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0.01, 0.05);
		return;
	}
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.4, 0.01, 0.2);
		return;
	}
	times.push_back(180);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.18, 0, 0.2);
		return;
	}
	times.push_back(700);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, -0.02, 0.8);
		return;
	}
	times.push_back(620);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.28, 0.007, 0.1);
		return;
	}
	times.push_back(600);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0., 0.007, -0.05);
		return;
	}
	times.push_back(350);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.08, 0.0, 0.1);
		return;
	}
	speed = 1.5;
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0.4, -0.05, -0.02);
		return;
	}
	times.push_back(150);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0.1, -0.15);
		return;
	}
	speed = 1;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0.05, -0.15);
		return;
	}
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0.09, 0);
		return;
	}
	speed = 0.4;
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0.1, 0);
		return;
	}
	times.push_back(1800);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0., 0);
		return;
	}
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, -0.08, 0);
		return;
	}
	speed = 0.8;
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, -0.08, 0);
		return;
	}
	speed = 1.6;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 3;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 6;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
	speed = 12;
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectArc170], speed, 0, 0, 0);
		return;
	}
}

void Application::animationTieFighter(unsigned long iteration) {
	std::vector<unsigned long> times;
	float speed = -0.6;
	times.push_back(2500);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, 0, 0);
		return;
	}
	speed = -1;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, 0, 0);
		return;
	}
	speed = -1.5;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, 0, 0);
		return;
	}
	speed = -2;
	times.push_back(200);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, 0, 0);
		return;
	}
	times.push_back(500);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, -0.4, -0.008, -0.2);
		return;
	}
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.1, 0.08, 0.2);
		return;
	}
	speed = -1;
	times.push_back(350);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, -0.1, -0.02, 0);
		return;
	}
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, -0.02, 0);
		return;
	}
	speed = -2;
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, -0.05, -0.1, -0.02);
		return;
	}
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, -0.54, 0, -0.2);
		return;
	}
	times.push_back(950);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.008, 0.028, 0.08);
		return;
	}
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, -0.6, -0.028, 0.3);
		return;
	}
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, -0.028, 0.3);
		return;
	}
	times.push_back(560);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, 0, 0.01);
		return;
	}
	times.push_back(420);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.44, 0, -0.2);
		return;
	}
	times.push_back(850);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0., 0, -0.4);
		return;
	}
	times.push_back(750);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.24, 0, -0.3);
		return;
	}
	times.push_back(850);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.0, 0.01, -0.1);
		return;
	}
	speed = -1.5;
	times.push_back(340);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.5, 0.02, -0.2);
		return;
	}
	speed = -1.2;
	times.push_back(300);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, -0.02, -0.2);
		return;
	}
	speed = -0.7;
	times.push_back(500);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0, -0.11, -0.1);
		return;
	}
	speed = -0.43;
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.0, -0.08, -0.01);
		return;
	}
	speed = -0.36;
	times.push_back(1800);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.0, 0., 0.);
		return;
	}
	speed = -0.5;
	times.push_back(800);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0., 0.07, 0);
		return;
	}
	times.push_back(400);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0., 0., 0);
		return;
	}
	times.push_back(2000);
	if (iteration <= iterationSum(times)){
		moveDemoSceneObject(m_sceneObjects[SceneObjectTieFighter_1], speed, 0.1, 0, -0.15);
		return;
	}

	
}	
	
float Application::interpole(int t, int max, float fo, float fc) {
	float alpha =  glm::clamp(float(t)/max, 0.f, 1.f);
	return alpha*fc + (1-alpha)*fo;
}


void Application::animationFollowCamera(unsigned long iteration) {
	std::vector<unsigned long> times;
	float speed = 0.6;
	times.push_back(2500);
	int t;
	if (iteration <= iterationSum(times)){
		moveFollowCamera(speed, 0, 0, 0);
		return;
	}
	speed = 1;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveFollowCamera(speed, 0, 0, 0);
		return;
	}
	speed = 1.5;
	times.push_back(100);
	if (iteration <= iterationSum(times)){
		moveFollowCamera(speed, 0, 0, 0);
		return;
	}
	speed = 2;
	times.push_back(250);
	if (iteration <= iterationSum(times)){
		moveFollowCamera(speed, 0, 0, 0);
		return;
	}
	times.push_back(186);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,186,0,0.015), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,40,0.015,0), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(130);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,130,0,0.02), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(280);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,20,0.02,0), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(220);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,250,0,0.0065), interpole(t,250,0,0.001), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,300,0.0065,-0.011), interpole(t,300,0.001,-0.006), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,20,-0.011,0), interpole(t,300,-0.006,0.011), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(440);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,440,0,-0.014), interpole(t,10,0.011,0), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(1000);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,4,-0.014,0), interpole(t,1000,0,-0.001), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,100,0,-0.012), interpole(t,100,-0.001,0.001), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(800);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,20,-0.012,0.),  interpole(t,400,0.001,0), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(420);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,420,0,0.014),  interpole(t,400,0,0.0002), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(900);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,20,0.014,0), interpole(t,900,0.0002,-0.0007), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,300,0,0.011), interpole(t,300,-0.0007,0.0018), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(250);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,50,0.011,0), interpole(t,100,0.0018,0), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,50,0,0.005), interpole(t,100,0,-0.0007), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(800);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(speed, interpole(t,20,0.005,0), interpole(t,10,-0.0007,0), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(300);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(interpole(t,300,speed,1.5), interpole(t,260,0.005,0.0125), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(500);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(interpole(t,500,1.5,0.5), interpole(t,40,0.0125,0), 0, 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(500);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(0.5, 0, interpole(t,500,0,0.0035), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(2400);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(interpole(t,1000,0.5,0.35), 0, interpole(t,300,0.0035,0.), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(500);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(interpole(t,500,0.35,0.5), 0, interpole(t,300,0.,-0.0028), 0);
			t++;
			return;
		}
		t = 0;
	}
	times.push_back(800);
	{
		static int t = 0;
		if (iteration <= iterationSum(times)){
			moveFollowCamera(interpole(t,800,0.5,0), 0, interpole(t,50,-0.0028,0), 0);
			t++;
			return;
		}
		t = 0;
	}
}

void Application::animationsetCamera(unsigned long iteration) {
	/*
	if (indexcam == 0)
		currentViewMatrix = viewController.getViewMatrix();
	else if (indexcam == 1)
		currentViewMatrix = followCamera.getViewMatrix();
	else if (indexcam == 2)
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].camera.getViewMatrix();
	else if (indexcam == 3)
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].OutCamera;
	else if (indexcam == 4)
		currentViewMatrix = m_sceneObjects[SceneObjectTieFighter_1].camera.getViewMatrix();
	else if (indexcam == 5)
		currentViewMatrix = innerArcCamera.getViewMatrix();
	else
		currentViewMatrix = innerTieCamera.getViewMatrix();
	return;*/
	
	
	if (iteration <= 350) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 850) {
		currentViewMatrix = innerArcCamera.getViewMatrix();
	} else if (iteration <= 1550) {
		currentViewMatrix =  m_sceneObjects[SceneObjectArc170].OutCamera;
	} else if (iteration <= 2420) {
		currentViewMatrix = innerArcCamera.getViewMatrix();
		currentViewMatrix = innerArcCamera.getViewMatrix();
	} else if (iteration <= 7900) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 8360) {
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].OutCamera;
	} else if (iteration <= 9450) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 9910) {
		currentViewMatrix = innerTieCamera.getViewMatrix();
	} else if (iteration <= 10700) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 11070) {
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].camera.getViewMatrix();
	} else if (iteration <= 11600) {
		currentViewMatrix = innerArcCamera.getViewMatrix();
	} else if (iteration <= 11900) {
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].OutCamera;
	} else if (iteration <= 12400) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 12950) {
		currentViewMatrix = innerTieCamera.getViewMatrix();
	} else if (iteration <= 14000) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 16400) {
		currentViewMatrix = followCamera.getViewMatrix();
	} else if (iteration <= 17200) {
		currentViewMatrix = innerArcCamera.getViewMatrix();
	} else {
		currentViewMatrix = m_sceneObjects[SceneObjectArc170].OutCamera;
	}
}

void Application::animationLights(unsigned long iteration) {
	if ((iteration/100) % 2 == 0) {
		PointLightIntensities[1] = glm::vec3(0);
	} else {
		PointLightIntensities[1] = glm::vec3(3000,0,0);
	}
	if (iteration < 2000 || iteration > 16000) {
		if ((iteration/100) % 2 == 0) {
			PointLightIntensities[2] = glm::vec3(0,500,0);
		} else {
			PointLightIntensities[2] = glm::vec3(0,1000,0);
		} 
	} else {
		if ((iteration/100) % 2 == 0) {
			PointLightIntensities[2] = glm::vec3(2000,0,0);
		} else {
			PointLightIntensities[2] = glm::vec3(1000,0,0);
		} 
	}
}

void Application::animationEffect(unsigned long iteration) {
	currentEffect = PostEffectNone;
	int fadetime = 200;
	if (iteration <= 350) {
		blackFadeAlpha = (iteration > fadetime) ? (1) : (float(iteration)/fadetime);
		currentEffect = PostEffectBlackFade;
	}
	int time = 17500;
	if (time <= iteration ) { 
		blackFadeAlpha = 1-float(iteration-time)/fadetime;
		currentEffect = PostEffectBlackFade;
	}
}

void Application::animationSceneObjects(unsigned long iteration) {
	animationArc170(iteration);
	animationTieFighter(iteration);
	animationFollowCamera(iteration);
	animationsetCamera(iteration);
	animationLights(iteration);
	animationEffect(iteration);
}

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };

    float DirLightPhiAngleDegrees = 90;
	float DirLightThetaAngleDegrees = 90;
	glm::vec3 DirectionalLightIntensity(1.f);

	const auto sceneDiagonalSize = glm::length(m_bboxMax - m_bboxMin);
    
    float sceneSize = sceneDiagonalSize;
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), float(m_nWindowWidth)/m_nWindowHeight, 0.005f * sceneDiagonalSize, sceneSize);  // near = 1% de la taille de la scene, far = 100%

    bool directionalSMDirty = true;
    
    float shadowMapBias = 0.01f;
	int shadowMapSampleCount = 19;
	float shadowMapSpreadCoeff = 90.f;
	float shadowMapSpreadInit = 0.00001*0.3;
	float shadowMapSpread = shadowMapSpreadInit*shadowMapSpreadCoeff;

	const auto begin_seconds = glfwGetTime();
	
	bool isPlaying = false;
	unsigned long iteration = 0;

	std::cout << "====================================================" << std::endl;
	std::cout << "Appuyez sur entree pour lancer l'animation" << std::endl;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
		
		if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_ENTER) && !isPlaying) {
			isPlaying = !isPlaying;
		}
		
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
		if (iteration%3 == 0) {
			directionalSMDirty = true;
		}
		
		const auto time = seconds - begin_seconds;
		
		if (isPlaying) {
			animationSceneObjects(iteration);
		} else {
			currentEffect = PostEffectBlackFade;
			blackFadeAlpha = 0;
		}
		glm::mat4 ViewMatrix =  (currentViewMatrix);

		geometryPass(ProjMatrix, ViewMatrix);

		shadingPass(
			DirectionalLightDirs, DirectionalLightIntensities,
			PointLightPositions, PointLightIntensities,
			DirLightDirection, DirectionalLightIntensity,
			ViewMatrix, glm::inverse(ViewMatrix),
			dirLightProjMatrix, dirLightViewMatrix,
			shadowMapBias, shadowMapSampleCount, shadowMapSpread
		);
        
        computePostEffect();  

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();
        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("Time : %f", time);
            ImGui::Text("Iteration : %ld", iteration);
            ImGui::Separator();
            if (ImGui::Button("Camera")) {
				indexcam = (indexcam + 1)%7;
			}
            if (ImGui::Button("Effect")) {
				currentEffect = static_cast<PostEffect>((currentEffect+1)%PostEffectCount);
			}
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
				ImGui::SliderFloat("Spread", &shadowMapSpreadCoeff, 0, 1000, "%f");
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
        
        if (isPlaying) {
			iteration++;
		}
		if (iteration == 18000) {
			isPlaying = false;
			iteration = 0;
			resetSceneObjects();
			std::cout << "Appuyez sur entree pour lancer l'animation" << std::endl;
		}
        
    }

    return 0;
}

Application::Application(int argc, char** argv):
	viewController(m_GLFWHandle.window()), //TODO rm

    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
	m_AssetsRootPath{ m_AppPath.parent_path() / "assets" }
  
{	
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	glEnable(GL_DEPTH_TEST);

	loadSceneObjects();
	
	m_default_material.Ka = glm::vec3(1.f);
	m_default_material.Kd = glm::vec3(1.f);
	m_default_material.Ks = glm::vec3(1.f);
	m_bboxMax = m_sceneObjects[0].objectData.bboxMax;
	m_bboxMin = m_sceneObjects[0].objectData.bboxMin;
	
	
	//TODO RM
	const auto sceneDiagonalSize = glm::length(m_bboxMax - m_bboxMin);
    viewController.setSpeed(sceneDiagonalSize * 0.1f); // 10% de la scene parcouru par seconde
    //........
    

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
	m_gammaCorrectionProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "gammaCorrect.cs.glsl" });
	m_blackFadeProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "blackFade.cs.glsl" });

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
	uGDepth = glGetUniformLocation(m_programShadingPass.glId(), "uGDepth");
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
	uGammaExponent = glGetUniformLocation(m_gammaCorrectionProgram.glId(), "uGammaExponent");
	uAlpha = glGetUniformLocation(m_blackFadeProgram.glId(), "uAlpha");
	
	uProjMat = glGetUniformLocation(m_programShadingPass.glId(), "uProjectionMat");
	uKernels = glGetUniformLocation(m_programShadingPass.glId(), "uKernels");
	uNoiseVec = glGetUniformLocation(m_programShadingPass.glId(), "uNoiseVec");
	uTexNoise = glGetUniformLocation(m_programShadingPass.glId(), "uTexNoise");

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

	
	//
	
	glGenTextures(1, &m_BeautyTexture);
	glBindTexture(GL_TEXTURE_2D, m_BeautyTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, m_nWindowWidth, m_nWindowHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &m_BeautyFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BeautyFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, m_BeautyTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	std::cerr << "Framebuffer pre-prerocessing status : " << (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) ==  GL_FRAMEBUFFER_COMPLETE) << std::endl;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

	glGenTextures(1, &m_GammaCorrectedBeautyTexture);
	glBindTexture(GL_TEXTURE_2D, m_GammaCorrectedBeautyTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, m_nWindowWidth, m_nWindowHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &m_GammaCorrectedBeautyFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GammaCorrectedBeautyFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, m_GammaCorrectedBeautyTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	std::cerr << "Framebuffer post-processing gamma correct status : " << (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) ==  GL_FRAMEBUFFER_COMPLETE) << std::endl;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 
	
	for (int i = 0; i < kernelSize; ++i) {
		glm::vec3 kernel = glm::vec3(
			RandomFloat(-1.0f, 1.0f),
			RandomFloat(-1.0f, 1.0f),
			RandomFloat(0.0f, 1.0f)
		);
		kernel = glm::normalize(kernel);
		kernel *= RandomFloat(0.0f, 1.0f);
		float scale = float(i) / float(kernelSize);
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		kernel *= scale;
		kernels.push_back(kernel[0]);
		kernels.push_back(kernel[1]);
		kernels.push_back(kernel[2]);
	}
	
	noise.resize(16);
	for (int i = 0; i < 16; ++i) {
		noise[i] = glm::vec3(
			RandomFloat(-1.0f, 1.0f),
			RandomFloat(-1.0f, 1.0f),
			0.f
			
		);
		noise[i] = glm::normalize(noise[i]);
	}
	
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, 4, 4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, noise.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenSamplers(1, &samplerNoise);
	glSamplerParameteri(samplerNoise, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNoise, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNoise, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerNoise, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
