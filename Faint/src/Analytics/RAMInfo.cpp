#include "hzpch.h"
#include "RAMInfo.h"

//#include <Windows.h>
#include <psapi.h>

namespace Faint::Hardware {

	void RamInfo::Update() {

		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);

		m_maxRAM = statex.ullTotalPhys / 1048576.0f;
		m_freeRAM = statex.ullTotalPhys / 1048576.0f;
		m_usedRAM = m_maxRAM - statex.ullTotalPhys / 1048576.0f;
	}

	float RamInfo::GetUsedRAM() {
		return m_usedRAM;
	}

	float RamInfo::GetFreeRAM() {
		return m_freeRAM;
	}

	float RamInfo::GetMaxRAM() {
		return m_maxRAM;
	}
}
