#include "AudioSystem.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

using namespace audioAL;


audioAL::AudioListener::AudioListener() : m_sources() {
	m_device = alcOpenDevice(nullptr);
	if (!m_device) {
		throw std::exception("OpenAL device is failed to open.");
	}

	m_context = alcCreateContext(m_device, NULL);
	if (!m_context) {
		throw std::exception("OpenAL context is failed to create.");
	}

	alcMakeContextCurrent(m_context);

	SetPosition({});
	SetRotation({});
}

audioAL::AudioListener::~AudioListener() {
	m_sources.clear();

	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

std::shared_ptr<AudioSource> audioAL::AudioListener::AddAudio(const char* filepath) {
	std::shared_ptr<AudioSource> source = std::make_shared<AudioSource>(filepath);
	m_sources.insert(source);
	return source;
}

void audioAL::AudioListener::SetPosition(lm::vec3 value) {
	m_position = value;
	
	alListener3f(AL_POSITION, value.x, value.y, value.z);
}

void audioAL::AudioListener::SetRotation(Quaternion value) {
	m_rotation = value;

	lm::vec3 orientations[2] = { m_rotation * lm::vec3{0, 0, -1}, m_rotation * lm::vec3{0, 1, 0} };

	alListenerfv(AL_ORIENTATION, &orientations[0].x);
}