#pragma once

namespace Faint {

	class Time {
	public:
		Time(float time = 0.0f) : m_time(time) {

		}

		operator float() const { return m_time; }

		float GetDeltaTime() const { return m_time; }
		float GetMilliseconds() const { return m_time * 1000.0f; }
		float GetFPS() const { return (float)(1.0 / m_time); }
	private:
		float m_time;
	};
}