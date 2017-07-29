#include <thread>
#include "Utility.hpp"

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				error = "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				error = "STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
		}
		static int t = 0;
		t++;
		if (t > 10)
			LOG(FATAL);
		LOG(ERROR) << "OpenGL error: " << error << " at " << file << ":" << line;
	}
	return errorCode;
}

LoopExecutionTimeController::LoopExecutionTimeController(duration delayLength) : delayLength(delayLength) {
	previousUpdate = clock::now();
}

LoopExecutionTimeController::~LoopExecutionTimeController() {

}

void LoopExecutionTimeController::SetDelayLength(duration length) {
	delayLength = length;
}

unsigned long long LoopExecutionTimeController::GetIterations() {
	return iterations;
}

void LoopExecutionTimeController::Update() {
	iterations++;
	auto timeToSleep = delayLength - GetDelta();
	if (timeToSleep.count()>0)
		std::this_thread::sleep_for(timeToSleep);
	previousUpdate = clock::now();
}

double LoopExecutionTimeController::GetDeltaMs() {
	auto now = clock::now();
	return duration(now-previousUpdate).count();
}

LoopExecutionTimeController::duration LoopExecutionTimeController::GetDelta() {
	auto now = clock::now();
	//std::cerr<<duration(now-previousUpdate).count()<<std::endl;
	return duration(now-previousUpdate);
}
