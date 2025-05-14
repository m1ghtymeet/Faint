#include <iostream>
#include <chrono>
#include <unordered_map>

namespace Faint::Hardware {
	
	class GPUInfo final {
	public:
		GPUInfo();
		float CalculateGPULoad();

	private:
		typedef int* (*NvAPI_QueryInterface_t)(unsigned int offset);
		typedef int* (*NvAPI_Initialize_t)();
		typedef int* (*NvAPI_EnumPhysicalGPUs_t)(int** handles, int* count);
		typedef int* (*NvAPI_GPU_GetUsages_t)(int* handle, unsigned int* usages);

		bool m_NvAPIReady = false;

		NvAPI_QueryInterface_t m_NvAPI_QueryInterface     = nullptr;
		NvAPI_Initialize_t m_NvAPI_Initialize             = nullptr;
		NvAPI_EnumPhysicalGPUs_t m_NvAPI_EnumPhysicalGPUs = nullptr;
		NvAPI_GPU_GetUsages_t m_NvAPI_GPU_GetUsages		  = nullptr;

		int m_gpuCount		         = 0;
		int* m_gpuHandles[64]		 = { nullptr };
		unsigned int m_gpuUsages[34] = { 0 };
	};
}