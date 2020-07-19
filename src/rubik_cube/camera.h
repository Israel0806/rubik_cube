#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using namespace glm;
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
	// Camera Attributes
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Constructor with vectors
	Camera (vec3 position = vec3 (0.0f, 0.0f, 0.0f), vec3 up = vec3 (0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front (vec3 (0.0f, 0.0f, -1.0f)), MovementSpeed (SPEED), MouseSensitivity (SENSITIVITY), Zoom (ZOOM) {
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors ();
	}
	// Constructor with scalar values
	Camera (float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front (vec3 (0.0f, 0.0f, -1.0f)), MovementSpeed (SPEED), MouseSensitivity (SENSITIVITY), Zoom (ZOOM) {
		Position = vec3 (posX, posY, posZ);
		WorldUp = vec3 (upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors ();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	mat4 GetViewMatrix () {
		return lookAt (Position, vec3 (0.0f), Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard (Camera_Movement direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		if ( direction == FORWARD )
			Position -= Position * velocity;
		if ( direction == BACKWARD )
			Position += Position * velocity;
		//if ( direction == LEFT )
		//	Position -= Right * velocity;
		//if ( direction == RIGHT )
		//	Position += Right * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement (float xoffset, float yoffset, bool rightClick, GLboolean constrainPitch = true) {
		if ( xoffset || yoffset ) {
			if ( rightClick ) {
				mat4 mat = mat4 (1.0f);
				mat = rotate (mat, radians (2.0f), vec3 (yoffset, -xoffset, 0.0f));
				vec3 vec = mat * vec4 (Position, 1.0f);
				Position = vec;
			}
		}

		/// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors ();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll (float yoffset) {
		if ( Zoom >= 1.0f && Zoom <= 45.0f )
			Zoom -= yoffset;
		if ( Zoom <= 1.0f )
			Zoom = 1.0f;
		if ( Zoom >= 45.0f )
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors () {
		// Calculate the new Front vector
		vec3 front;
		front.x = cos (radians (Yaw)) * cos (radians (Pitch));
		front.y = sin (radians (Pitch));
		front.z = sin (radians (Yaw)) * cos (radians (Pitch));
		Front = normalize (front);
		// Also re-calculate the Right and Up vector
		Right = normalize (cross (Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = normalize (cross (Right, Front));
	}
};
#endif

