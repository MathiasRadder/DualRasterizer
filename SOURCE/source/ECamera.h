/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/

#pragma once
#include "EMath.h"

namespace Elite
{
	class Camera
	{
	public:

		Camera(const FPoint3& position = { 0.f, 0.f, -10.f }, const FVector3& viewForward = { 0.f, 0.f, 1.f }, float fovAngle = 60.f);
		~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(float elapsedSec);
		FPoint3 GetPosition()const ;
		const FMatrix4& GetWorldToView() const { return m_WorldToView; };
		const FMatrix4& GetViewToWorld() const { return m_ViewToWorld; };
		const FMatrix4& GetProjectionMatrix() const { return m_ProjectionMatrix; };
		void CreateProjectionMatrix(float farDistance, float nearDistance, float aspectRatio);
		void HardwareRendering(bool hardwareRendering);

		const float GetFov() const { return m_Fov; }


	private:
		void CalculateLookAt();

		float m_Fov{};

		const float m_KeyboardMoveSensitivity{ 1.f };
		const float m_KeyboardMoveMultiplier{ 10.f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };
		const float m_MouseMoveMultiplier{ 10.f };


		FPoint2 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		FPoint3 m_RelativeTranslation{};

		FPoint3 m_Position{};
		const FVector3 m_ViewForward{};

		FMatrix4 m_WorldToView{};
		FMatrix4 m_ViewToWorld{};
		FMatrix4 m_ProjectionMatrix;

		float m_Far;
		float m_Near;
		float m_AspectRation;
		bool m_RenderingHardware;
	};
}
