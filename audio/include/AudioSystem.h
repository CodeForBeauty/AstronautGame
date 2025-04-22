#pragma once

#include "AudioSource.h"
#include <memory>
#include <unordered_set>

#include "Types.h"
#include "lm/lm.h"

#include "AL/alc.h"


namespace audioAL {
	class AudioListener {
	public:
		AudioListener();
		~AudioListener();

		std::shared_ptr<AudioSource> AddAudio(const char* filepath);

		void SetPosition(lm::vec3 value);
		void SetRotation(Quaternion value);

	protected:
		std::unordered_set<std::shared_ptr<AudioSource>> m_sources;

		lm::vec3 m_position;
		Quaternion m_rotation;

		ALCdevice* m_device;
		ALCcontext* m_context;
	};
}