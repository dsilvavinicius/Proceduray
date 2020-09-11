#pragma once

#include "Camera.h"
#include <memory.h>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;
	using namespace Math;
	using CameraPtr = shared_ptr<Camera>;

	class InputManager
	{
	public:
		enum MouseButton
		{
			LEFT
		};

		InputManager() : m_mousePos(0.f, 0.f), m_mousePrevPos(0.f, 0.f) {}

		void newMousePos(const XMFLOAT2& pos) { m_mousePrevPos = m_mousePos; m_mousePos = pos; }
		XMFLOAT2 getMouseDelta() const { return XMFLOAT2(m_mousePos.x - m_mousePrevPos.x, m_mousePos.y - m_mousePrevPos.y); }
		void setMouseButton(MouseButton button, bool status) { m_mouseState[button] = status; }
		void setKey(unsigned char key, bool status) { m_keyState[key] = status; }

		const unordered_map<MouseButton, bool> getMouseState() const { return m_mouseState; }
		const unordered_map<unsigned char, bool> getKeyState() const { return m_keyState; }

	private:
		XMFLOAT2 m_mousePos;
		XMFLOAT2 m_mousePrevPos;
		unordered_map<MouseButton, bool> m_mouseState;
		unordered_map<unsigned char, bool> m_keyState;
	};

	/** Camera controller. Controls a camera from the mini engine. */
	class CamController
	{
	public:
		CamController(CameraPtr cam) : m_cam(cam) {}

		void Update(float deltaT, const InputManager& input)
		{
			auto mouseState = input.getMouseState();
			if (mouseState.find(InputManager::LEFT) != mouseState.end() && mouseState.at(InputManager::LEFT))
			{
				// Rotation
				XMFLOAT2 delta = input.getMouseDelta();
				auto currentQuaternion = m_cam->GetRotation();
				Quaternion q(delta.y * 0.0001f, delta.x * 0.0001f, 0.f);

				m_cam->SetRotation(currentQuaternion * q);
			}

			// Translation
			{
				auto keyState = input.getKeyState();
				if (keyState.find('W') != keyState.end() && keyState.at('W'))
				{
					m_cam->SetPosition(m_cam->GetPosition() + Normalize(m_cam->GetForwardVec()) * deltaT * 100.f);
				}

				if (keyState.find('S') != keyState.end() && keyState.at('S'))
				{
					m_cam->SetPosition(m_cam->GetPosition() - Normalize(m_cam->GetForwardVec()) * deltaT * 100.f);
				}

				if (keyState.find('D') != keyState.end() && keyState.at('D'))
				{
					m_cam->SetPosition(m_cam->GetPosition() + Normalize(m_cam->GetRightVec()) * deltaT * 100.f);
				}

				if (keyState.find('A') != keyState.end() && keyState.at('A'))
				{
					m_cam->SetPosition(m_cam->GetPosition() - Normalize(m_cam->GetRightVec()) * deltaT * 100.f);
				}

				if (keyState.find('E') != keyState.end() && keyState.at('E'))
				{
					m_cam->SetPosition(m_cam->GetPosition() + Normalize(m_cam->GetUpVec()) * deltaT * 100.f);
				}

				if (keyState.find('Q') != keyState.end() && keyState.at('Q'))
				{
					m_cam->SetPosition(m_cam->GetPosition() - Normalize(m_cam->GetUpVec()) * deltaT * 100.f);
				}
			}

			m_cam->Update();
		}

	private:
		CameraPtr m_cam;
	};
}