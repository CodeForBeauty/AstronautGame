#pragma once

#include "Types.h"
#include "Events.hpp"
#include <lm/lm.h>

namespace physixx {
	enum PositionEvent {
		PositionChangeEvent
	};
	enum RotationEvent {
		RotationChangeEvent
	};

	enum CollisionEvent {
		CollisionOverlap,
		CollisionTouch,
	};

	class ColliderBox;
	class ColliderBody;

	struct CollisionData {
		float MTV;
		lm::vec3 Normal;
		lm::vec3 Position;
		bool IsOvelapping;
		ColliderBody* Other;
	};

	const static float AirDencity = 1.1455f;

	class ColliderBody {
	public:
		ColliderBody(float mass, lm::vec3 inertia);

		void AddForce(lm::vec3 dir, float strength);
		void AddTorque(lm::vec3 axis, float angle);
		void AddForceAt(lm::vec3 pos, lm::vec3 dir, float strength);
		void AddImpulseAt(lm::vec3 pos, lm::vec3 dir);

		void SetVelocity(lm::vec3 velocity);

		virtual void Update(float deltaTime, lm::vec3 gravity);

		virtual bool CheckCollision(ColliderBody const& b, CollisionData& data) const = 0;
		virtual bool CheckCollision(ColliderBox const& b, CollisionData& data) const = 0;

		lm::vec3 GetPosition() const;
		lm::vec3 GetLocalPosition() const;
		void SetPosition(const lm::vec3& newPos);
		void Move(const lm::vec3& offset);
		void SetLocalPos(const lm::vec3& newPos);
		void MoveLocal(const lm::vec3& offset);

		Quaternion GetRotation() const;
		Quaternion GetLocalRotation() const;
		void SetRotation(Quaternion newRot);
		void Rotate(Quaternion offset);
		void SetLocalRot(Quaternion newRot);
		void RotateLocalRot(Quaternion offset);

		lm::vec3 GetForward() const;
		lm::vec3 GetRight() const;
		lm::vec3 GetUp() const;

		void SetHasGravity(bool value);
		void SetColliding(bool value);
		void SetSimulated(bool value);
		void SetCanRotate(bool value);

		bool GetSimulated() const;

		void SetDragCoefficient(float value);
		void SetBounciness(float value);

		void MultVelocity(const lm::vec3& mult);

		void Bounce(lm::vec3 pos, lm::vec3 normal, ColliderBody& other);

		void SetParent(ColliderBody* parent);

		Event<PositionEvent, void(lm::vec3)> OnPosition;
		Event<RotationEvent, void(Quaternion)> OnRotation;

		Event<CollisionEvent, void(CollisionData)> OnCollision;

	protected:
		ColliderBody* m_parent;
		Event<PositionEvent, void(lm::vec3)>::EventHandler m_posHandle;
		Event<RotationEvent, void(Quaternion)>::EventHandler m_rotHandle;

		bool m_hasGravity;
		bool m_canCollide;
		bool m_isSimulated;
		bool m_canRotate;

		lm::vec3 m_velocity;
		lm::vec3 m_angularVel;
		lm::vec3 m_force;
		lm::vec3 m_acceleration;
		lm::vec3 m_torque;

		float m_mass;
		lm::vec3 m_inertia;
		float m_bounciness;

		lm::vec3 m_position;
		lm::vec3 m_localPos;
		Quaternion m_rotation;
		Quaternion m_localRot;
		lm::vec3 m_forward, m_right, m_up;

		float m_dragCoeff;
		lm::vec3 m_area;

	private:
		void UpdateAxes();
	};

	class ColliderBox : public ColliderBody {
	public:
		ColliderBox(float mass, lm::vec3 size = { 1, 1, 1 }, lm::vec3 position = { 0, 0, 0 });

		virtual bool CheckCollision(ColliderBody const& b, CollisionData& data) const override;
		virtual bool CheckCollision(ColliderBox const& b, CollisionData& data) const override;

		
	protected:
		lm::vec3 m_size;
	private:
		bool GetSeparatingPlane(const lm::vec3& RPos, const lm::vec3& Plane, const ColliderBox& box1, const ColliderBox& box2, CollisionData& data) const;
	};
}