#pragma once

#include "lm/lm.h"


namespace audioAL {
	class AudioSource {
	public:
		AudioSource(const char* filepath);
		~AudioSource();

		void LoadFile(const char* filepath);

		void SetLooping(bool value);
		void SetPitch(float value);
		void SetGain(float value);
		void SetPosition(lm::vec3 value);
		void SetVelocity(lm::vec3 value);

		void Play() const;
		void Stop() const;

	protected:
		unsigned int m_buffer;
		unsigned int m_source;

		bool m_isLooping;
		float m_pitch;
		float m_gain;
		lm::vec3 m_position;
		lm::vec3 m_velocity;
	};
}