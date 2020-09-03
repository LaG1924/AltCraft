#include "../Audio.hpp"
#include "../AssetManager.hpp"

#include <string>
#include <easylogging++.h>

#include "al.h"
#include "alc.h"

ALCdevice *device;
ALCcontext *context;

bool alCheckError_(const char *file, int line) noexcept;
#define alCheckError() alCheckError_(__FILE__, __LINE__)

bool alcCheckError_(const char *file, int line) noexcept;
#define alcCheckError() alcCheckError_(__FILE__, __LINE__)

bool alCheckError_(const char *file, int line) noexcept {
	ALenum errorCode = alGetError();
	if (errorCode != AL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case AL_INVALID_NAME:
				error = "INVALID_NAME";
				break;
			case AL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case AL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case AL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case AL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
		}
		LOG(ERROR) << "OpenAL error: " << error << " at " << file << ":" << line;
		return true;
	} else
		return false;
}

bool alcCheckError_(const char *file, int line) noexcept {
	ALenum errorCode = alGetError();
	if (errorCode != ALC_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case ALC_INVALID_DEVICE:
				error = "INVALID_DEVICE";
				break;
			case ALC_INVALID_CONTEXT:
				error = "INVALID_CONTEXT";
				break;
			case ALC_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case ALC_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case ALC_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
		}
		LOG(ERROR) << "OpenAL context error: " << error << " at " << file << ":" << line;
		return true;
		assert(0);
	} else
		return false;
}

void Audio::Deinit() noexcept {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void Audio::Init() {
	device = alcOpenDevice(NULL);
	alcCheckError();
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context))
		alcCheckError();
}

void Audio::UpdateListener(Vector3<float> direction, Vector3<float> up, Vector3<float> pos, Vector3<float> vel) noexcept {
	ALfloat listPos[6] = {direction.x, direction.y, direction.z, up.x, up.y, up.z};

	alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
	alCheckError();
	alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	alCheckError();
	alListenerfv(AL_ORIENTATION, listPos);
	alCheckError();
}
