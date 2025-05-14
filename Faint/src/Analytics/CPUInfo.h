#include <iostream>
#include <chrono>
#include <unordered_map>

#include "Debug/Log.h"

namespace Faint::Hardware {
	
	class CPUInfo final {
	public:
		float CalculateCPULoad();

	private:
		float CalculateCPULoad(uint64_t idleTicks, uint64_t totalTicks);

		uint64_t m_cpuPreviousTotalTicks = 0;
		uint64_t m_cpuPreviousIdleTicks = 0;
	};
}