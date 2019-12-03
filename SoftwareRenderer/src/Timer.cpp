#include <ctime>
#include <chrono>

class Timer {
	std::chrono::steady_clock::time_point m_StartTime;
	std::chrono::steady_clock::time_point m_EndTime;
	std::chrono::duration<float, std::milli> m_Duration;

public:
	Timer() {
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	float milliElapsed() {
		m_EndTime = std::chrono::high_resolution_clock::now();
		m_Duration = m_EndTime - m_StartTime;
		return m_Duration.count();
	}

	float secondsElapsed() {
		m_EndTime = std::chrono::high_resolution_clock::now();
		m_Duration = m_EndTime - m_StartTime;
		return m_Duration.count() / 1000;
	}

	void ResetStartTime() {
		m_StartTime = std::chrono::high_resolution_clock::now();
	}
};