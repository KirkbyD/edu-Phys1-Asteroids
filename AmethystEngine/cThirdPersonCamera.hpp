#pragma once
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "iGameObject.hpp"
#include "cModelObject.hpp"

class cThirdPersonCamera
{
private:
	float yaw;
	float pitch;
	float followDistance;

	glm::vec3 eye;			//defaults  (0.0f, 0.0f, 0.0f)
	glm::vec3 upVector;		//defaults	(0.0f, 1.0f, 0.0f)
	glm::vec3 front;		//defaults  (0.0f, 0.0f, 1.0f)
	glm::vec3 right;
	glm::vec3 up;
	glm::mat4 viewMatrix;
	
	iGameObject* followObject;

public:
	cThirdPersonCamera();

	glm::vec3 GetEye();
	glm::vec3 GetUpVector();
	glm::vec3 GetFront();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::mat4 GetViewMatrix();

	void SetUpVector(glm::vec3 upVector);
	void SetObject(iGameObject* newFollowTarget);
	void SetFollowDistance(float distanceMultiplier);

	void MoveForBack(float speed);
	void MoveUpDown(float speed);
	void MoveStrafe(float speed);
	void Move();
	void MoveFront();

	void OffsetYaw(float offset);
	void OffsetPitch(float offset);
};