#include "PhysixxBody.h"

#include <math.h>

using namespace physixx;
using namespace lm;


ColliderBody::ColliderBody(float mass, vec3 inertia) 
	: m_mass(mass), m_inertia(inertia), m_hasGravity(true), m_canCollide(true), m_isSimulated(true), m_canRotate(true),
	m_dragCoeff(10.0f), m_bounciness(1.0f),
	m_forward(0, 0, -1), m_right(1, 0, 0), m_up(0, 1, 0), 
	m_parent(nullptr), m_posHandle(), m_rotHandle() {

}

void physixx::ColliderBody::AddForce(lm::vec3 dir, float strength) {
	m_force += dir * strength;
}

void physixx::ColliderBody::AddTorque(vec3 axis, float angle) {
	if (m_canRotate) {
		m_torque += axis * -angle;
	}
}

void physixx::ColliderBody::AddForceAt(lm::vec3 pos, lm::vec3 dir, float strength) {
	AddForce(dir, strength);
	vec3 localPos = pos - m_position;
	vec3 axis = cross(localPos, dir * strength);
	float sizeSq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
	if (sizeSq > 0.05f) {
		AddTorque(axis, 0.03f);
	}
}

void physixx::ColliderBody::AddImpulseAt(lm::vec3 pos, lm::vec3 dir) {
	m_velocity += dir / m_mass;
	m_angularVel += cross(pos - m_position, dir * 0.05f) / m_inertia;
}

// Set velocity of the body, ignoring previous velocity
void physixx::ColliderBody::SetVelocity(lm::vec3 velocity) {
	m_velocity = velocity;
}

void physixx::ColliderBody::Update(float deltaTime, vec3 gravity) {
	if (!m_isSimulated) {
		m_force = {};
		m_torque = {};
		return;
	}

	// Integrate velocity using Verlet integration
	float deltaSq = deltaTime * deltaTime;
	m_position = m_position + m_velocity * deltaTime + m_acceleration * (deltaSq * 0.5f);
	lm::vec3 acceleration = m_force / m_mass + gravity * m_hasGravity;
	m_velocity = m_velocity + (acceleration + m_acceleration) * (deltaTime * 0.5f);
	m_acceleration = acceleration;

	if (magnitude(m_velocity) > 0) {
		OnPosition.Invoke(PositionChangeEvent, m_position);
	}
	// Euler integration for angular velocity
	m_angularVel += m_torque / m_inertia;
	
	if (magnitude(m_angularVel) > 0.1e-3f) {
		lm::vec3 angularAcc = m_rotation * m_angularVel * deltaTime * deltaTime;
		float length = magnitude(angularAcc);
		float half = length * 0.5f;
		float sin = sinf(half);
		float cos = cosf(half);
		
		Quaternion q = Quaternion(angularAcc.x * sin, angularAcc.y * sin, angularAcc.z * sin, length * cos);

		Rotate(q);
	}

	// Angular velocity loses force every second
	m_angularVel -= m_angularVel * (deltaTime * 10);

	float speedSq = m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y + m_velocity.z * m_velocity.z;

	m_torque = {};

	if (speedSq > 0.1e-5f) {
		// Add air resistance to velocity
		lm::vec3 globalArea = m_rotation * m_area;
		globalArea = { fabs(globalArea.x), fabs(globalArea.y), fabs(globalArea.z) };
		lm::vec3 drag = 1.0f / 2.0f * AirDencity * speedSq * m_dragCoeff * globalArea;
		m_force = (deltaTime * 2) * drag * -lm::normalize(m_velocity);
	}
	else {
		m_force = {};
	}
}

lm::vec3 physixx::ColliderBody::GetPosition() const {
	return m_position;
}

lm::vec3 physixx::ColliderBody::GetLocalPosition() const {
	return m_localPos;
}

void physixx::ColliderBody::SetPosition(const lm::vec3& newPos) {
	m_position = newPos;

	OnPosition.Invoke(PositionChangeEvent, m_position);
}

void physixx::ColliderBody::Move(const lm::vec3& offset) {
	m_position += offset;

	OnPosition.Invoke(PositionChangeEvent, m_position);
}

void physixx::ColliderBody::SetLocalPos(const lm::vec3& newPos) {
	m_localPos = newPos;
}

void physixx::ColliderBody::MoveLocal(const lm::vec3& offset) {
	m_localPos += offset;
}

Quaternion physixx::ColliderBody::GetRotation() const {
	return m_rotation;
}

Quaternion physixx::ColliderBody::GetLocalRotation() const {
	return Quaternion();
}

void physixx::ColliderBody::SetRotation(Quaternion newRot) {
	m_rotation = newRot;
	m_rotation.Normalize();

	UpdateAxes();

	OnRotation.Invoke(RotationChangeEvent, m_rotation);
}

void physixx::ColliderBody::Rotate(Quaternion offset) {
	m_rotation = offset * m_rotation;
	m_rotation.Normalize();

	UpdateAxes();

	OnRotation.Invoke(RotationChangeEvent, m_rotation);
}

void physixx::ColliderBody::SetLocalRot(Quaternion newRot) {
	m_localRot = newRot;
}

void physixx::ColliderBody::RotateLocalRot(Quaternion offset) {
	m_localRot = offset * m_localRot;
}

lm::vec3 physixx::ColliderBody::GetForward() const {
	return m_forward;
}

lm::vec3 physixx::ColliderBody::GetRight() const {
	return m_right;
}

lm::vec3 physixx::ColliderBody::GetUp() const {
	return m_up;
}

void physixx::ColliderBody::SetHasGravity(bool value) {
	m_hasGravity = value;
}

void physixx::ColliderBody::SetColliding(bool value) {
	m_canCollide = value;
}

void physixx::ColliderBody::SetSimulated(bool value) {
	m_isSimulated = value;
}

void physixx::ColliderBody::SetCanRotate(bool value) {
	m_canRotate = value;
}

bool physixx::ColliderBody::GetSimulated() const {
	return m_isSimulated;
}

void physixx::ColliderBody::SetDragCoefficient(float value) {
	m_dragCoeff = value;
}

void physixx::ColliderBody::SetBounciness(float value) {
	m_bounciness = value;
}

void physixx::ColliderBody::MultVelocity(const lm::vec3& mult) {
	m_velocity *= mult;
}

// Bounce from collision
void physixx::ColliderBody::Bounce(lm::vec3 pos, lm::vec3 normal, ColliderBody& other) {
	if (!m_isSimulated) {
		return;
	}
	float speed = magnitude(m_velocity);

	vec3 R = pos - m_position;

	float strength = (m_mass + other.m_mass) * (speed) * 80 * m_bounciness;
	AddForce(normal, strength);
	AddTorque(cross(R, normal * strength), 0.05f);

	m_velocity *= 1 - normal;
	m_angularVel *= 1 - normal;
}

void physixx::ColliderBody::SetParent(ColliderBody* parent) {
	if (m_parent != nullptr) {
		m_parent->OnPosition.RemoveListener(m_posHandle);
		m_parent->OnRotation.RemoveListener(m_rotHandle);
		m_parent = nullptr;
	}
	m_parent = parent;
	if (parent == nullptr) {
		return;
	}
	m_posHandle = m_parent->OnPosition.AddListener(PositionChangeEvent, [this](vec3 pos) {
		SetPosition(m_parent->m_rotation.GetMatrix() * m_localPos + pos);
		});
	m_rotHandle = m_parent->OnRotation.AddListener(RotationChangeEvent, [this](Quaternion rot) {
		SetRotation(m_localRot * rot);
		});
}

void physixx::ColliderBody::UpdateAxes() {
	lm::mat3 rot = m_rotation.GetMatrix();

	m_right = lm::normalize(rot.x);
	m_up = lm::normalize(rot.y);
	m_forward = lm::normalize(-rot.z);
}

physixx::ColliderBox::ColliderBox(float mass, vec3 size, vec3 position) : ColliderBody(mass, 
	{
		1.0f / 12 * mass * (size.y * size.y + size.z * size.z),
		1.0f / 12 * mass * (size.x * size.x + size.z * size.z),
		1.0f / 12 * mass * (size.y * size.y + size.x * size.x),
	}), m_size(size) {
	m_position = position;
	m_area = { size.y * size.z, size.x * size.z, size.x * size.y };
	OnPosition.Invoke(PositionChangeEvent, m_position);
}

bool physixx::ColliderBox::CheckCollision(ColliderBody const& b, CollisionData& data) const {
	if (!m_canCollide) {
		return false;
	}
	return b.CheckCollision(*this, data);
}

// Check collision on selected axis and generate collision data
bool ColliderBox::GetSeparatingPlane(const vec3& RPos, const vec3& Plane, const ColliderBox& box1, const ColliderBox& box2, CollisionData& data) const {
	if (fabs(Plane.x) <= 0.01f && fabs(Plane.y) <= 0.01f && fabs(Plane.z) <= 0.01f) {
		return false;
	}

	float proj = (std::fabs(dot((box1.m_right * box1.m_size.x), Plane)) +
		std::fabs(dot((box1.m_up * box1.m_size.y), Plane)) +
		std::fabs(dot((box1.m_forward * box1.m_size.z), Plane)) +
		std::fabs(dot((box2.m_right * box2.m_size.x), Plane)) +
		std::fabs(dot((box2.m_up * box2.m_size.y), Plane)) +
		std::fabs(dot((box2.m_forward * box2.m_size.z), Plane)));
	float RPosXPlane = dot(RPos, Plane);
	float dist = std::fabs(RPosXPlane);

	float seperation = proj - dist;
	if (fabs(seperation) < fabs(data.MTV)) {
		data.MTV = seperation;
		data.Normal = Plane;

		if (RPosXPlane < 0) {
			data.Normal = -Plane;
		}
		float normalSize = magnitude(data.Normal);
		if (normalSize != 0) {
			data.Normal = normalize(data.Normal);
		}
		else {
			data.Normal = {};
		}

		lm::vec3 vertex = box2.m_size;
		if (dot(box2.m_right, data.Normal) < 0) vertex.x = -vertex.x;
		if (dot(box2.m_up, data.Normal) > 0) vertex.y = -vertex.y;
		if (dot(box2.m_forward, data.Normal) >= 0) vertex.z = -vertex.z;
		
		data.Position = box2.m_rotation.GetMatrix() * vertex + box2.m_position;

		data.IsOvelapping = fabs(seperation) > 0.0005f;

		data.Other = (ColliderBody*)&box1;
	}

	return (dist > proj);
}

// Check collision using SAT
bool physixx::ColliderBox::CheckCollision(ColliderBox const& b, CollisionData& data) const {
	if (!m_canCollide) {
		return false;
	}
	lm::vec3 RPos = b.m_position - m_position;

	data.MTV = HUGE;

	return !(GetSeparatingPlane(RPos, m_right,                       *this, b, data) ||
			 GetSeparatingPlane(RPos, m_up,                          *this, b, data) ||
			 GetSeparatingPlane(RPos, m_forward,                     *this, b, data) ||
			 GetSeparatingPlane(RPos, b.m_right,                     *this, b, data) ||
			 GetSeparatingPlane(RPos, b.m_up,                        *this, b, data) ||
			 GetSeparatingPlane(RPos, b.m_forward,                   *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_right, b.m_right),     *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_right, b.m_up),        *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_right, b.m_forward),   *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_up, b.m_right),        *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_up, b.m_up),           *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_up, b.m_forward),      *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_forward, b.m_right),   *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_forward, b.m_up),      *this, b, data) ||
			 GetSeparatingPlane(RPos, cross(m_forward, b.m_forward), *this, b, data));
}
