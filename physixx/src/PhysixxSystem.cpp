#include "PhysixxSystem.h"

using namespace physixx;

PhysixxSystem::PhysixxSystem() : m_bodies(), m_toRemove(), Gravity(0, -9.81f, 0), m_simSteps(4) {

}

void physixx::PhysixxSystem::AddBody(std::shared_ptr<ColliderBody> body) {
	m_bodies.push_back(body);
}

void physixx::PhysixxSystem::RemoveBody(std::shared_ptr<ColliderBody> body) {
	m_toRemove.insert(body.get());
}

void physixx::PhysixxSystem::RemoveBody(ColliderBody* body) {
	m_toRemove.insert(body);
}

void physixx::PhysixxSystem::Update(float deltaTime) {
	float stepDelta = deltaTime / (float)m_simSteps;
	// Update all of the bodies in the system
	for (int i = 0; i < m_bodies.size(); i++) {
		m_bodies[i]->Update(deltaTime, Gravity);
		CollisionData data = {};
		for (unsigned int step = 0; step < m_simSteps; step++) {
			for (int j = 0; j < m_bodies.size(); j++) {
				if (j == i) {
					continue;
				}
				if (m_bodies[i]->CheckCollision(*m_bodies[j].get(), data)) {
					if (data.IsOvelapping) {
						m_bodies[i]->OnCollision.Invoke(CollisionOverlap, data);
						CollisionData data1 = data;
						data1.Other = m_bodies[i].get();
						m_bodies[j]->OnCollision.Invoke(CollisionOverlap, data1);
					}
					else {
						m_bodies[i]->OnCollision.Invoke(CollisionTouch, data);
					}

					if (!m_bodies[i]->GetSimulated()) {
						continue;
					}

					if (data.IsOvelapping) {
						m_bodies[i]->Bounce(data.Position,  data.Normal, *m_bodies[j].get());
						m_bodies[j]->Bounce(data.Position, -data.Normal, *m_bodies[i].get());
					}
					else {
						m_bodies[i]->MultVelocity((1 - data.Normal));
					}

					m_bodies[i]->Move(data.Normal * (data.MTV * 2));
				}
			}
		}
	}

	// Remove all bodies qued up for removal
	for (ColliderBody* body : m_toRemove) {
		auto findPos = std::find_if(m_bodies.begin(), m_bodies.end(), [body](const std::shared_ptr<ColliderBody>& other) { return other.get() == body; });
		if (findPos != m_bodies.end()) {
			m_bodies.erase(findPos);
		}
	}

	m_toRemove.clear();
}
