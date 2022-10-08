#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// a classe camera apesar de muito modificada foi feita em cima da classe disponibilizada pelo site learnopengl.com
// FONTE
// https://learnopengl.com/Getting-started/Camera

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	float ScreenRatio;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : 
		Front(glm::vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED), 
		MouseSensitivity(SENSITIVITY), 
		Zoom(ZOOM), 
		ScreenRatio(16.0f / 9.0f)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : 
		Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
		MovementSpeed(SPEED), 
		MouseSensitivity(SENSITIVITY), 
		Zoom(ZOOM), 
		ScreenRatio(16.0f / 9.0f)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	static glm::mat4 Matrix(
		float m00, float m01, float m02, float m03, // LINHA 1
		float m10, float m11, float m12, float m13, // LINHA 2
		float m20, float m21, float m22, float m23, // LINHA 3
		float m30, float m31, float m32, float m33  // LINHA 4
	)
	{
		return glm::mat4(
			m00, m10, m20, m30, // COLUNA 1
			m01, m11, m21, m31, // COLUNA 2
			m02, m12, m22, m32, // COLUNA 3
			m03, m13, m23, m33  // COLUNA 4
		);
	}

	static glm::mat4 Matrix_Camera_View(glm::vec3 pos, glm::vec3 view_vector, glm::vec3 up_vector)
	{
		glm::vec3 w = -view_vector;
		glm::vec3 u = glm::cross(up_vector, w);

		// Normalizamos os vetores u e w
		w = glm::normalize(w);
		u = glm::normalize(u);

		glm::vec3 v = glm::cross(w, u);

		glm::vec4 origin_o = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 position_c = glm::vec4(pos, 1.0f);

		float ux = u.x;
		float uy = u.y;
		float uz = u.z;
		float vx = v.x;
		float vy = v.y;
		float vz = v.z;
		float wx = w.x;
		float wy = w.y;
		float wz = w.z;

		glm::mat4 m = Matrix(
			ux, uy, uz, -glm::dot(glm::vec4(u, 0.0f), position_c - origin_o),
			vx, vy, vz, -glm::dot(glm::vec4(v, 0.0f), position_c - origin_o),
			wx, wy, wz, -glm::dot(glm::vec4(w, 0.0f), position_c - origin_o),
			0.0f, 0.0f, 0.0f, 1.0f
		);

		return m;
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() const
	{
		glm::vec3 w = -Front;
		glm::vec3 u = glm::cross(WorldUp, w);

		// Normalizamos os vetores u e w
		w = glm::normalize(w);
		u = glm::normalize(u);

		glm::vec3 v = glm::cross(w, u);

		glm::vec4 origin_o = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 position_c = glm::vec4(Position, 1.0f);

		float ux = u.x;
		float uy = u.y;
		float uz = u.z;
		float vx = v.x;
		float vy = v.y;
		float vz = v.z;
		float wx = w.x;
		float wy = w.y;
		float wz = w.z;

		glm::mat4 m = Matrix(
			ux, uy, uz, -glm::dot(glm::vec4(u, 0.0f), position_c - origin_o),
			vx, vy, vz, -glm::dot(glm::vec4(v, 0.0f), position_c - origin_o),
			wx, wy, wz, -glm::dot(glm::vec4(w, 0.0f), position_c - origin_o),
			0.0f, 0.0f, 0.0f, 1.0f
		);

		return m;
	}

	// Matriz de projeção paralela ortográfica
	static glm::mat4 Matrix_Orthographic(float l, float r, float b, float t, float n, float f)
	{
		glm::mat4 M = Matrix(
			2.0f / (r - l), 0.0f, 0.0f, -(r + l) / (r - l),
			0.0f, 2.0f / (t - b), 0.0f, -(t + b) / (t - b),
			0.0f, 0.0f, 2.0f / (f - n), -(f + n) / (f - n),
			0.0f, 0.0f, 0.0f, 1.0f
		);

		return M;
	}

	glm::mat4 GetProjectionMatrix() const {
		float n = 0.1f;
		float f = 500.0f;
		float field_of_view = 3.141592 / 3.0f;
		float t = fabs(n) * tanf(glm::radians(ZOOM) / 2.0f);
		float b = -t;
		float r = t * ScreenRatio;
		float l = -r;

		glm::mat4 P = Matrix(
			n, 0.0f, 0.0f, 0.0f,
			0.0f, n, 0.0f, 0.0f,
			0.0f, 0.0f, n + f, -f * n,
			0.0f, 0.0f, 1.0f, 0.0f
		);

		// A matriz M é a mesma computada acima em Matrix_Orthographic().
		glm::mat4 M = Matrix_Orthographic(l, r, b, t, n, f);
		
		glm::mat4 m = M * P;
		m[2][3] *= -1;
		m[2][2] *= -1;
		
		return m;
	}


	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	virtual void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	virtual void ProcessMouseScroll(float yoffset) {}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	virtual void ProcessKeyboard(Camera_Movement direction, float deltaTime) {}

	// calculates the front vector from the Camera's (updated) Euler Angles
	virtual void updateCameraVectors() {}
};

class FreeCamera : Camera {
	using Camera::Camera;

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 170.0f)
			Zoom = 170.0f;
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position += WorldUp * velocity;
		if (direction == DOWN)
			Position -= WorldUp * velocity;
	}

	void updateCameraVectors() {
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

class LookAtCamera : Camera {
	using Camera::Camera;

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		distanceFromTarget -= yoffset;

		updateCameraVectors();
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		updateCameraVectors();
		return; 
	}

	void updateCameraVectors() {
		glm::vec3 pos;
		pos.x = cos(glm::radians(Yaw)) * cos(glm::radians(-Pitch));
		pos.y = sin(glm::radians(-Pitch));
		pos.z = sin(glm::radians(Yaw)) * cos(glm::radians(-Pitch));
	
		Position = pos * distanceFromTarget + target;

		glm::vec3 front = normalize(target - Position);

		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

public:
	glm::vec3 target = glm::vec3(0.0f,0.0f,0.0f);
	float distanceFromTarget = 20.0f;
};
#endif
