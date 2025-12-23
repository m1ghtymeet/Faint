#pragma once

#include <chrono>

namespace Moon::Tools {

	// Time class to handle time-related operations
	class Time {
	public:
		void Update();

		float GetFramerate();

		float GetDeltaTime();

		float GetTimeSinceStart();

		void Scale(float p_ceoff);

		void SetTimeScale(float p_timeScale);

		operator float() const { return __DELTA_TIME; }
	private:
		void Initialize();

		std::chrono::steady_clock::time_point __START_TIME;
		std::chrono::steady_clock::time_point __LAST_TIME;
		std::chrono::steady_clock::time_point __CURRENT_TIME;
		std::chrono::duration<double> __ELAPSED;

		bool  __INITIALIZED = false;
		float __TIME_SCALE = 1.0f;
		float __DELTA_TIME = 0.0f;
		float __TIME_SINCE_START = 0.0f;
	};
}