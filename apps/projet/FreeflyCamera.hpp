#ifndef __FREEFLY_CAMERA_H__
#define __FREEFLY_CAMERA_H__

#include "Camera.hpp"
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/load_obj.hpp>
#include <glmlv/ViewController.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

class FreeflyCamera : public Camera {

private:

	glm::vec3 m_Position;
	float m_fPhi, m_fTheta, m_fPsi;
	glm::vec3 m_FrontVector, m_LeftVector, m_UpVector;

	glm::mat4 m_ViewMatrix = glm::mat4(1);

	void computeDirectionVectors();

public:

	FreeflyCamera();

	FreeflyCamera(glm::vec3 Position, float phi, float theta);

	
	void setHorizontalAngle(float a);

	void setVerticalAngle(float a);
	
	void setFrontAngle(float a);

	void setPosition(glm::vec3 position);

	
	void rotateHorizontalAngle(float a);
	
	void rotateVerticalAngle(float a);
	
	void rotateFrontAngle(float a);
	
	void moveFront(float d);
	
	void moveUp(float d);
	
	void moveLeft(float d);
	

	glm::mat4 getViewMatrix() const override;

};

#endif
