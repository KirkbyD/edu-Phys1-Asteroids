#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "cCallbacks.hpp"
#include "cCommandManager.hpp"
#include "Globals.hpp"

#include <stdio.h>
#include <iostream>

bool isShiftKeyDownByAlone(int mods);
bool isCtrlKeyDownByAlone(int mods);
float devSpeed = 300.0f;
float maxSpeed = 2.0;
bool g_MidtermLaserBlast = false;

// ADDITIONALLY CHANGE THE DEVELOPER CONSOLE TO USE A SECONDARY WINDOW WITH THE STUFF FROM OSCARS CLASS
// FUCNTION FOR ACTUALLY ISSUING THE COMMAND (THIS IS ESSENTIALLY THE MEDIATOR)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	cCommandManager* pCmdMngr = cCommandManager::GetCommandManager();
	//float accelSpeed = 180.0f * deltaTime;
	//float rotationSpeed = 1.0f * deltaTime;
	float devMoveSpeed = devSpeed * deltaTime;

	if (!isDevCon && (key == GLFW_KEY_F && action == GLFW_PRESS)) {
		//pCmdMngr->IssueCommand("light.visible toggle");
	}

	// DEVELOPER CONSOLE ACTIVATION CHECK
	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		if (!isDevCon) {
			isDevCon = true;
			glfwSetCharCallback(window, character_callback);
		}
		else {
			isDevCon = false;
			glfwSetCharCallback(window, NULL);
		}
	}

	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveForBack(devMoveSpeed);
	}
	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveForBack(-devMoveSpeed);
	}
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveStrafe(devMoveSpeed);
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveStrafe(-devMoveSpeed);
	}
	if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveUpDown(devMoveSpeed);
	}
	if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		mainCamera.MoveUpDown(-devMoveSpeed);
	}
	// Fire Everything!
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		g_MidtermLaserBlast = !g_MidtermLaserBlast;
	}

	// DEVELOPER CONSOLE ENTER CHARACTER
	if (isDevCon && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		pCmdMngr->IssueCommand(cmdstr);
		cmdstr = "";
	}

	// DEVELOPER CONSOLE DELETE CHARACTER
	if (isDevCon && key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (!cmdstr.empty()) {
			cmdstr.pop_back();
		}
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

bool isShiftKeyDownByAlone(int mods) {
	if (mods == GLFW_MOD_SHIFT)
		return true;
	return false;
}

bool isCtrlKeyDownByAlone(int mods) {
	if (mods == GLFW_MOD_CONTROL)
		return true;
	return false;
}

void character_callback(GLFWwindow* window, unsigned codepoint) {
	// STUFF FOR THE DEVELOPER CONSOLE
	if ((char)codepoint != '`')
		cmdstr.push_back((char)codepoint);
}