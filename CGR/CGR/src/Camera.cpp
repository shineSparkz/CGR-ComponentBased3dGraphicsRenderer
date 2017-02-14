#include "Camera.h"
#include "Input.h"

#include "EventManager.h"
#include "KeyEvent.h"
#include "Screen.h"

Camera::Camera() :
	position(0.0f),
	up(0.0f),
	forward(0.0f),
	right(0.0f),
	fov(0.0f),
	aspect(0.0f),
	near(0.0f),
	far(0.0f)
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->AttachEvent(KEY_EVENT, *this);
	}
}

void Camera::Update()
{
	const float& dt = Time::DeltaTime();
	velocity = Vec3(0.0f);

	int num_dirs_pressed = ((rk ^ lk) ? 1 : 0) +
		((fk ^ bk) ? 1 : 0);

	if (lk ^ rk)
	{
		velocity = lk ? velocity - (right * speed) : velocity + (right * speed);
	}

	if (fk ^ bk)
	{
		velocity = fk ?
			velocity + forward * speed :
			velocity - forward * speed;
	}
	
	float hscreenWidth =  (float)Screen::Instance()->ScreenWidth() * 0.5f;
	float hscreenHeight = (float)Screen::Instance()->ScreenHeight() * 0.5f;
	float mouseX = (float)Mouse::Instance()->PosX();
	float mouseY = (float)Mouse::Instance()->PosY();

	// Mouse orient
	yaw   += mouseSpeed * dt * (hscreenWidth  - mouseX);
	pitch += mouseSpeed * dt * (hscreenHeight - mouseY);

	this->forward = //Vec3(0, 0, -1);
		Vec3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw));

	this->right = //Vec3(1, 0, 0);
		Vec3(sinf(yaw - 3.14f / 2.0f), 0, cosf(yaw - 3.14f / 2.0f));

	this->up = glm::cross(right, forward);

	// Update position
	position += (this->velocity * dt);
	projection = glm::perspective(fov, aspect, near, far);
	view = glm::lookAt(position, position + forward, up);

	Mouse::Instance()->SetMousePosition(hscreenWidth, hscreenHeight);

}

void Camera::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
		case KEY_EVENT:
		{
			KeyEvent* ke = (KeyEvent*)e->GetData();

			if (ke)
			{
				if (ke->key == GLFW_KEY_W &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT) )
				{
					fk = true;
				}
				else
				{
					fk = false;
				}
				
				if (ke->key == GLFW_KEY_S &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					bk = true;
				}
				else
				{
					bk = false;
				}

				if (ke->key == GLFW_KEY_A &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					lk = true;
				}
				else
				{
					lk = false;
				}

				if (ke->key == GLFW_KEY_D &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					rk = true;
				}
				else
				{
					rk = false;
				}
			}
		}
	}
}