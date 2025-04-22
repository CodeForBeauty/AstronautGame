#pragma once

#include "PhysixxBody.h"
#include <vector>
#include <memory>


namespace physixx {
	class PhysixxSystem {
	public:
		PhysixxSystem();

		template<class BodyType, typename... ArgTypes>
		std::shared_ptr<BodyType> AddBody(ArgTypes... args);
		void AddBody(std::shared_ptr<ColliderBody> body);
		void RemoveBody(std::shared_ptr<ColliderBody> body);
		void RemoveBody(ColliderBody* body);

		void Update(float deltaTime);

		lm::vec3 Gravity;

	protected:
		std::vector<std::shared_ptr<ColliderBody>> m_bodies;
		std::unordered_set<ColliderBody*> m_toRemove;

		unsigned int m_simSteps;

	};

	template<class BodyType, typename ...ArgTypes>
	inline std::shared_ptr<BodyType> PhysixxSystem::AddBody(ArgTypes ...args) {
		std::shared_ptr<BodyType> body = std::make_shared<BodyType>(args...);

		m_bodies.push_back(body);

		return body;
	}
}