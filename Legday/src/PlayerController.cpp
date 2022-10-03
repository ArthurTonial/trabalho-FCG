#include "PlayerController.h"



void PlayerController::initComponent() {
	deltaTime = -1.0f;
}

void PlayerController::execute() {

	if (!is_active) {
		acceleration = vec3(0.0f, 0.0f, 0.0f);
		velocity = vec3(0.0f, 0.0f, 0.0f);
		return;
	}

	keyboardInput();

	if (deltaTime > 0.0f) {
		deltaTime = glfwGetTime() - lastTime;
		velocity += acceleration * acc * deltaTime;

		float sp = length(velocity);
		if (sp > 0.0f) {
			velocity = normalize(velocity) * std::min(sp, speed);
			velocity -= velocity * damp * deltaTime;
		}

		objectTransform->position += (objectTransform->rotation * velocity) * deltaTime;
		lastTime = glfwGetTime();
	}
	else {
		lastTime = glfwGetTime();
		deltaTime = 1.0f;
	}

}

void PlayerController::endComponent() {

}

void PlayerController::keyboardInput() {
	acceleration = vec3(0.0f, 0.0f, 0.0f);

	if (*(is_pressed + KEYS::W)) {
		acceleration += vec3(0.0f, 0.0f, 1.0f);
	}
	if (*(is_pressed + KEYS::A)) {
		acceleration += vec3(1.0f, 0.0f, 0.0f);
	}
	if (*(is_pressed + KEYS::S)) {
		acceleration += vec3(0.0f, 0.0f, -1.0f);
	}
	if (*(is_pressed + KEYS::D)) {
		acceleration += vec3(-1.0f, 0.0f, 0.0f);
	}

	if (length(acceleration) > 0.0f) {
		acceleration = normalize(acceleration);
	}
}