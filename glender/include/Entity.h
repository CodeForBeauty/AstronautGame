#pragma once

#include "Events.hpp"
#include "Types.h"

#include <lm/lm.h>
#include <vector>
#include <set>
#include <unordered_set>
#include <memory>


namespace glender {
	struct TransformData {
		lm::vec3 Position;
		Quaternion Rotation;
		lm::vec3 Scale;
	};

	enum TransformEvents {
		TransformEventPosition,
		TransformEventRotation,
		TransformEventScale,
		TransformEventChange
	};

	enum NameEvents {
		NameChangeEvent,
	};

	class Transform {
	public:
		Transform();

		virtual void SetRotation(const lm::vec3& rotation);
		virtual void Rotate(const lm::vec3& offset);
		virtual void SetRotation(const Quaternion& rotation);
		virtual void Rotate(const Quaternion& offset);
		virtual void SetPosition(const lm::vec3& position);
		virtual void Move(const lm::vec3& offset);
		virtual void SetScale(const lm::vec3& scale);

		virtual void SetTransform(const TransformData& value);

		lm::vec3 GetPosition() const;
		Quaternion GetRotation() const;
		lm::vec3 GetEulerRotation() const;
		lm::vec3 GetScale() const;

		lm::mat4 WorldMatrix;
		lm::mat4 LocalMatrix;
		
		std::unordered_set<Transform*> Children;
		Transform* Parent;

		void AddChild(Transform* child);
		void SetParent(Transform* parent);
		void RemoveChild(Transform* child);
		void UpdateTransform();

		lm::vec3 GetForward() const;
		lm::vec3 GetRight() const;
		lm::vec3 GetUp() const;

		void SetName(const std::string& newName);
		std::string GetName() const;

		Event<TransformEvents, void(lm::vec3)> OnChangeTransform;
		Event<NameEvents, void(std::string)> OnChangeName;

	protected:
		lm::vec3 m_position;
		Quaternion m_rotation;
		lm::vec3 m_scale;

		lm::vec3 m_forward;
		lm::vec3 m_right;
		lm::vec3 m_up;

		std::string m_name;

	private:

	};

	class Entity {
	public:
		Entity();

		Transform EntityTransform;

	protected:

	private:

	};
}