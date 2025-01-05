#include "../lib/AL/al.h"
#include "../lib/AL/alc.h"
#include "soundcommon.h"

#include <iostream>//for the FIRST TIME EVER iostream isn't the first include (not)


#define DR_WAV_IMPLEMENTATION
#include "../lib/dr/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "../lib/dr/dr_mp3.h"

#define DR_FLAC_IMPLEMENTATION
#include "../lib/dr/dr_flac.h"


class Sound{
    public:
        Sound(const char* path){
            //default to wav because i don't have time for the other ones
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 totalSampleCount;
            int16_t* pSampleData = drwav_open_file_and_read_pcm_frames_s16(path, &channels, &sampleRate, &totalSampleCount, nullptr);
            //./resources/space.wav
            if (!pSampleData) {
                std::cerr << "Failed to load WAV" << std::endl;
                alcDestroyContext(context);
                alcCloseDevice(aldevice);
            }

            ALenum format;
            if (channels == 1) {
                format = AL_FORMAT_MONO16;
            } else if (channels == 2) {
                format = AL_FORMAT_STEREO16;
            } else {
                std::cerr << "Unsupported number of channels: " << channels << std::endl;
                drwav_free(pSampleData, nullptr);
                alcDestroyContext(context);
                alcCloseDevice(aldevice);
            }

            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, pSampleData, totalSampleCount * channels * sizeof(int16_t), sampleRate);

            drwav_free(pSampleData, nullptr);

            alGenSources(1, &source);
            alSourcei(source, AL_BUFFER, buffer);
        }
        ~Sound(){
            //now what to put here is the real question
        }

        void setGain(float gain){
            alSourcef(source, AL_GAIN, gain);
        }
        void setPitch(float pitch){
            alSourcef(source, AL_GAIN, pitch);
        }
        void setLoop(bool loops){
            alSourcei(source, AL_LOOPING, loops?AL_TRUE:AL_FALSE);
        }
        void play(){
            alSourcePlay(source);
        }
        

    private:
        ALuint buffer;
        ALuint source;
};