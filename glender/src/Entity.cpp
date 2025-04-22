#include "Entity.h"
#include "Logging.h"

using namespace glender;


Transform::Transform() : m_position(0, 0, 0), m_rotation(0, 0, 0, 1), m_scale(1, 1, 1), Parent(nullptr),
			m_forward(0.0f, 0.0f, -1.0f), m_right(1.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f), Children() {
	UpdateTransform();
}

void Transform::SetRotation(const lm::vec3& rotation) {
	m_rotation = Quaternion(rotation * lm::PIrad);
	m_rotation.Normalize();
	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventRotation, rotation);
}

void Transform::Rotate(const lm::vec3& offset) {
	Quaternion change = Quaternion(offset * lm::PIrad);
	change.Normalize();
	m_rotation = change * m_rotation;
	m_rotation.Normalize();

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventRotation, m_rotation.GetEuler());
}

void Transform::SetRotation(const Quaternion& rotation) {
	m_rotation = rotation;
	m_rotation.Normalize();

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventRotation, m_rotation.GetEuler());
}

void Transform::Rotate(const Quaternion& offset) {
	m_rotation = offset * m_rotation;
	m_rotation.Normalize();

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventRotation, m_rotation.GetEuler());
}

void Transform::SetPosition(const lm::vec3& position) {
	m_position = position;

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventPosition, m_position);
}

void Transform::Move(const lm::vec3& offset) {
	m_position += offset;

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventPosition, m_position);
}

void Transform::SetScale(const lm::vec3& scale) {
	m_scale = scale;

	UpdateTransform();

	OnChangeTransform.Invoke(TransformEventScale, m_scale);
}

void Transform::SetTransform(const TransformData& value) {
	m_position = value.Position;
	m_rotation = value.Rotation;
	m_rotation.Normalize();
	m_scale = value.Scale;

	UpdateTransform();
}

lm::vec3 Transform::GetPosition() const {
	return m_position;
}

Quaternion Transform::GetRotation() const {
	return m_rotation;
}

lm::vec3 Transform::GetEulerRotation() const {
	return m_rotation.GetEuler();
}

lm::vec3 Transform::GetScale() const {
	return m_scale;
}

void Transform::AddChild(Transform* child) {
	if (child == nullptr) {
		GlenderLog(LogTypeWarning, "Trying to add nullptr as child.");
		return;
	}
	Children.insert(child);
	child->Parent = this;

	child->UpdateTransform();
}

void Transform::SetParent(Transform* parent) {
	if (parent == nullptr) {
		if (Parent) {
			Parent->RemoveChild(this);
		}
		Parent = nullptr;
		UpdateTransform();
		return;
	}
	Transform* current = this;
	if (Parent) {
		Parent->RemoveChild(current);
	}
	Parent = parent;
	parent->AddChild(current);
}

void Transform::RemoveChild(Transform* child) {
	Children.erase(child);
}

void Transform::UpdateTransform() {
	lm::mat4 pos = lm::position3d(m_position);
	pos.w.w = 1.0f;
	lm::mat4 rot = m_rotation.GetMatrix();
	rot.w.w = 1.0f;
	lm::mat4 scale = { {m_scale.x, 0.0f, 0.0f, 0.0f},
						{0.0f, m_scale.y, 0.0f, 0.0f},
						{0.0f, 0.0f, m_scale.z, 0.0f},
						{0.0f, 0.0f, 0.0f, 1.0f} };
	LocalMatrix = scale * rot * pos;
	WorldMatrix = LocalMatrix;
	
	if (Parent != nullptr) {
		WorldMatrix = WorldMatrix * Parent->WorldMatrix;
	}

	m_forward = lm::normalize(lm::vec3{ -WorldMatrix.z.x, -WorldMatrix.z.y, -WorldMatrix.z.z });
	m_right = lm::normalize(lm::vec3{ WorldMatrix.x.x, WorldMatrix.x.y, WorldMatrix.x.z });
	m_up = lm::normalize(lm::vec3{ WorldMatrix.y.x, WorldMatrix.y.y, WorldMatrix.y.z });
	
	for (Transform* child : Children) {
		child->UpdateTransform();
	}

	OnChangeTransform.Invoke(TransformEventChange, {});
}

lm::vec3 Transform::GetForward() const {
	return m_forward;
}

lm::vec3 Transform::GetRight() const {
	return m_right;
}

lm::vec3 Transform::GetUp() const {
	return m_up;
}

void Transform::SetName(const std::string& newName) {
	m_name = newName;

	OnChangeName.Invoke(NameChangeEvent, newName);
}

std::string Transform::GetName() const {
	return m_name;
}

Entity::Entity() {
	EntityTransform.SetName("Entity");
}