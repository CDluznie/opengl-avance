#include "FreeflyCamera.hpp"

#include <iostream>

// Pour palier aux soucis de macro M_PI sous windows , vu que c'est une norme POSX
constexpr double pi() { return std::atan2(0, -1); }

FreeflyCamera::FreeflyCamera() :
		Camera(),
		m_Position(glm::vec3(0, 0, 0)),
		m_fPhi(static_cast<float>(pi())),
		m_fTheta(0),
		m_LeftVector(0, 0, 0),
		m_FrontVector(0, 0, 0),
		m_UpVector(0, 0, 0) {


}

FreeflyCamera::FreeflyCamera(glm::vec3 Position, float phi, float theta) :
		Camera(),
		m_Position(Position),
		m_LeftVector(0, 0, 0),
		m_FrontVector(0, 0, 0),
		m_UpVector(0, 0, 0) 
{
	setHorizontalAngle(glm::radians(phi));
	setVerticalAngle(glm::radians(theta));
	setFrontAngle(0);
}

void FreeflyCamera::setHorizontalAngle(float a) {
	m_fPhi = (a);
	computeDirectionVectors();
}

void FreeflyCamera::setVerticalAngle(float a) {
	m_fTheta = (a);
	computeDirectionVectors();
}

void FreeflyCamera::setFrontAngle(float a) {
	m_fPsi = (a);
	computeDirectionVectors();
}

void FreeflyCamera::rotateHorizontalAngle(float a) {
	m_fPhi += (a);
	computeDirectionVectors();
}

void FreeflyCamera::rotateVerticalAngle(float a) {
	m_fTheta += (a);
	computeDirectionVectors();
}

void FreeflyCamera::rotateFrontAngle(float a) {
	m_fPsi += (a);
	computeDirectionVectors();
}

void FreeflyCamera::setPosition(glm::vec3 position) {
	m_Position = position;
}

void FreeflyCamera::moveFront(float d) {
	m_Position += d*m_FrontVector;
}

void FreeflyCamera::moveUp(float d) {
	m_Position += d*m_UpVector;
}

void FreeflyCamera::moveLeft(float d) {
	m_Position += d*m_LeftVector;
}



void FreeflyCamera::computeDirectionVectors() {
	m_LeftVector = glm::vec3(sin(m_fPhi + pi() / 2),
	                         0,
	                         cos(m_fPhi + pi() / 2));
	m_FrontVector = glm::vec3(std::cos(m_fTheta) * std::sin(m_fPhi),
	                          std::sin(m_fTheta),
	                          std::cos(m_fTheta) * std::cos(m_fPhi));
	m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
	m_ViewMatrix = glm::rotate(glm::mat4(1.f), (-m_fPsi), glm::vec3(0,0,1))*glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
}

glm::mat4 FreeflyCamera::getViewMatrix() const {
	auto ViewMatrix = glm::rotate(glm::mat4(1.f), (-m_fPsi), glm::vec3(0,0,1))*glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
	return ViewMatrix;

}
