#include "AudioSource.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "sndfile.h"
#include "AL/al.h"
#include "AL/alext.h"


enum FormatType {
    Int16,
    Float,
    IMA4,
    MSADPCM
};

audioAL::AudioSource::AudioSource(const char* filepath) : m_buffer(AL_NONE), m_source(AL_NONE), 
        m_isLooping(false), m_gain(1), m_pitch(1) {
    alGenBuffers(1, &m_buffer);
    alGenSources(1, &m_source);
    LoadFile(filepath);
}

audioAL::AudioSource::~AudioSource() {
    alSourceStop(m_source);
    alSourcei(m_source, AL_BUFFER, (ALint)0);
    alDeleteBuffers(1, &m_buffer);
    alDeleteSources(1, &m_source);
}

void audioAL::AudioSource::LoadFile(const char* filepath) {
    enum FormatType sample_format = Int16;
    ALint byteblockalign = 0;
    ALint splblockalign = 0;
    sf_count_t num_frames;
    ALenum err, format;
    ALsizei num_bytes;
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    void* membuf;

    /* Open the audio file and check that it's usable. */
    sndfile = sf_open(filepath, SFM_READ, &sfinfo);
    if (!sndfile) {
        fprintf(stderr, "Could not open audio in %s: %s\n", filepath, sf_strerror(sndfile));
        return;
    }
    if (sfinfo.frames < 1) {
        fprintf(stderr, "Bad sample count in %s (%" PRId64 ")\n", filepath, sfinfo.frames);
        sf_close(sndfile);
        return;
    }

    /* Detect a suitable format to load. Formats like Vorbis and Opus use float
     * natively, so load as float to avoid clipping when possible. Formats
     * larger than 16-bit can also use float to preserve a bit more precision.
     */
    switch ((sfinfo.format & SF_FORMAT_SUBMASK)) {
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_FLOAT:
    case SF_FORMAT_DOUBLE:
    case SF_FORMAT_VORBIS:
    case SF_FORMAT_OPUS:
    case SF_FORMAT_ALAC_20:
    case SF_FORMAT_ALAC_24:
    case SF_FORMAT_ALAC_32:
    case 0x0080/*SF_FORMAT_MPEG_LAYER_I*/:
    case 0x0081/*SF_FORMAT_MPEG_LAYER_II*/:
    case 0x0082/*SF_FORMAT_MPEG_LAYER_III*/:
        if (alIsExtensionPresent("AL_EXT_FLOAT32"))
            sample_format = Float;
        break;
    case SF_FORMAT_IMA_ADPCM:
        /* ADPCM formats require setting a block alignment as specified in the
         * file, which needs to be read from the wave 'fmt ' chunk manually
         * since libsndfile doesn't provide it in a format-agnostic way.
         */
        if (sfinfo.channels <= 2 && (sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
            && alIsExtensionPresent("AL_EXT_IMA4")
            && alIsExtensionPresent("AL_SOFT_block_alignment"))
            sample_format = IMA4;
        break;
    case SF_FORMAT_MS_ADPCM:
        if (sfinfo.channels <= 2 && (sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
            && alIsExtensionPresent("AL_SOFT_MSADPCM")
            && alIsExtensionPresent("AL_SOFT_block_alignment"))
            sample_format = MSADPCM;
        break;
    }

    if (sample_format == IMA4 || sample_format == MSADPCM) {
        /* For ADPCM, lookup the wave file's "fmt " chunk, which is a
         * WAVEFORMATEX-based structure for the audio format.
         */
        SF_CHUNK_INFO inf = { "fmt ", 4, 0, NULL };
        SF_CHUNK_ITERATOR* iter = sf_get_chunk_iterator(sndfile, &inf);

        /* If there's an issue getting the chunk or block alignment, load as
         * 16-bit and have libsndfile do the conversion.
         */
        if (!iter || sf_get_chunk_size(iter, &inf) != SF_ERR_NO_ERROR || inf.datalen < 14)
            sample_format = Int16;
        else {
            ALubyte* fmtbuf = static_cast<ALubyte*>(calloc(inf.datalen, 1));
            inf.data = fmtbuf;
            if (sf_get_chunk_data(iter, &inf) != SF_ERR_NO_ERROR)
                sample_format = Int16;
            else {
                /* Read the nBlockAlign field, and convert from bytes- to
                 * samples-per-block (verifying it's valid by converting back
                 * and comparing to the original value).
                 */
                byteblockalign = fmtbuf[12] | (fmtbuf[13] << 8);
                if (sample_format == IMA4) {
                    splblockalign = (byteblockalign / sfinfo.channels - 4) / 4 * 8 + 1;
                    if (splblockalign < 1
                        || ((splblockalign - 1) / 2 + 4) * sfinfo.channels != byteblockalign)
                        sample_format = Int16;
                }
                else {
                    splblockalign = (byteblockalign / sfinfo.channels - 7) * 2 + 2;
                    if (splblockalign < 2
                        || ((splblockalign - 2) / 2 + 7) * sfinfo.channels != byteblockalign)
                        sample_format = Int16;
                }
            }
            free(fmtbuf);
        }
    }

    if (sample_format == Int16) {
        splblockalign = 1;
        byteblockalign = sfinfo.channels * 2;
    }
    else if (sample_format == Float) {
        splblockalign = 1;
        byteblockalign = sfinfo.channels * 4;
    }

    /* Figure out the OpenAL format from the file and desired sample type. */
    format = AL_NONE;
    if (sfinfo.channels == 1) {
        if (sample_format == Int16)
            format = AL_FORMAT_MONO16;
        else if (sample_format == Float)
            format = AL_FORMAT_MONO_FLOAT32;
        else if (sample_format == IMA4)
            format = AL_FORMAT_MONO_IMA4;
        else if (sample_format == MSADPCM)
            format = AL_FORMAT_MONO_MSADPCM_SOFT;
    }
    else if (sfinfo.channels == 2) {
        if (sample_format == Int16)
            format = AL_FORMAT_STEREO16;
        else if (sample_format == Float)
            format = AL_FORMAT_STEREO_FLOAT32;
        else if (sample_format == IMA4)
            format = AL_FORMAT_STEREO_IMA4;
        else if (sample_format == MSADPCM)
            format = AL_FORMAT_STEREO_MSADPCM_SOFT;
    }
    else if (sfinfo.channels == 3) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT) {
            if (sample_format == Int16)
                format = AL_FORMAT_BFORMAT2D_16;
            else if (sample_format == Float)
                format = AL_FORMAT_BFORMAT2D_FLOAT32;
        }
    }
    else if (sfinfo.channels == 4) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT) {
            if (sample_format == Int16)
                format = AL_FORMAT_BFORMAT3D_16;
            else if (sample_format == Float)
                format = AL_FORMAT_BFORMAT3D_FLOAT32;
        }
    }
    if (!format) {
        fprintf(stderr, "Unsupported channel count: %d\n", sfinfo.channels);
        sf_close(sndfile);
        return;
    }

    if (sfinfo.frames / splblockalign > (sf_count_t)(INT_MAX / byteblockalign)) {
        fprintf(stderr, "Too many samples in %s (%" PRId64 ")\n", filepath, sfinfo.frames);
        sf_close(sndfile);
        return;
    }

    /* Decode the whole audio file to a buffer. */
    membuf = malloc((size_t)(sfinfo.frames / splblockalign * byteblockalign));

    if (sample_format == Int16)
        num_frames = sf_readf_short(sndfile, static_cast<short*>(membuf), sfinfo.frames);
    else if (sample_format == Float)
        num_frames = sf_readf_float(sndfile, static_cast<float*>(membuf), sfinfo.frames);
    else {
        sf_count_t count = sfinfo.frames / splblockalign * byteblockalign;
        num_frames = sf_read_raw(sndfile, membuf, count);
        if (num_frames > 0)
            num_frames = num_frames / byteblockalign * splblockalign;
    }
    if (num_frames < 1) {
        free(membuf);
        sf_close(sndfile);
        fprintf(stderr, "Failed to read samples in %s (%" PRId64 ")\n", filepath, num_frames);
        return;
    }
    num_bytes = (ALsizei)(num_frames / splblockalign * byteblockalign);

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file.
     */
    
    alSourcei(m_source, AL_BUFFER, (ALint)0);

    if (splblockalign > 1)
        alBufferi(m_buffer, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, splblockalign);
    alBufferData(m_buffer, format, membuf, num_bytes, sfinfo.samplerate);

    alSourcei(m_source, AL_BUFFER, (ALint)m_buffer);

    free(membuf);
    sf_close(sndfile);

    /* Check if an error occurred, and clean up if so. */
    err = alGetError();
    if (err != AL_NO_ERROR) {
        fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
        if (m_buffer && alIsBuffer(m_buffer)) {
            alDeleteBuffers(1, &m_buffer);
            m_buffer = AL_NONE;
        }
        return;
    }

    alSourcei(m_source, AL_BUFFER, (ALint)m_buffer);
}

void audioAL::AudioSource::SetLooping(bool value) {
    m_isLooping = value;

    alSourcei(m_source, AL_LOOPING, value);
}

void audioAL::AudioSource::SetPitch(float value) {
    m_pitch = value;

    alSourcef(m_source, AL_GAIN, value);
}

void audioAL::AudioSource::SetGain(float value) {
    m_gain = value;

    alSourcef(m_source, AL_GAIN, value);
}

void audioAL::AudioSource::SetPosition(lm::vec3 value) {
    m_position = value;

    alSource3f(m_source, AL_POSITION, value.x, value.y, value.z);
}

void audioAL::AudioSource::SetVelocity(lm::vec3 value) {
    m_velocity = value;
    
    alSource3f(m_source, AL_VELOCITY, value.x, value.y, value.z);
}

void audioAL::AudioSource::Play() const {
    alSourcePlay(m_source);
}

void audioAL::AudioSource::Stop() const {
    alSourceStop(m_source);
}