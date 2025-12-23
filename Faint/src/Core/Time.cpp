#include "Time.h"

void Moon::Tools::Time::Initialize() {

	__DELTA_TIME = 0.0f;

	__START_TIME = std::chrono::steady_clock::now();
	__CURRENT_TIME = __START_TIME;
	__LAST_TIME = __START_TIME;

	__INITIALIZED = true;
}


void Moon::Tools::Time::Update() {

	__LAST_TIME = __CURRENT_TIME;
	__CURRENT_TIME = std::chrono::steady_clock::now();
	__ELAPSED = __CURRENT_TIME - __LAST_TIME;

	if (__INITIALIZED) {
		__DELTA_TIME = __ELAPSED.count() > 0.1 ? 0.1f : static_cast<float>(__ELAPSED.count());
		__TIME_SINCE_START += __DELTA_TIME * __TIME_SCALE;
	}
	else
		Initialize();
}

float Moon::Tools::Time::GetFramerate() {
	return 1.0f / (__DELTA_TIME);
}

float Moon::Tools::Time::GetDeltaTime() {
	return __DELTA_TIME * __TIME_SCALE;
}

float Moon::Tools::Time::GetTimeSinceStart() {
	return __TIME_SINCE_START;
}

void Moon::Tools::Time::Scale(float p_ceoff) {
	__TIME_SCALE *= p_ceoff;
}

void Moon::Tools::Time::SetTimeScale(float p_timeScale) {
	__TIME_SCALE = p_timeScale;
}