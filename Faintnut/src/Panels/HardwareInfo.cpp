#include "HardwareInfo.h"
#include "UI/ImUI.h"

#include <Engine.h>
#include <Analytics/CPUInfo.h>
#include <Analytics/GPUInfo.h>

void HardwareInfo::Draw() {

	if (ImGui::Begin("Hardware Info")) {
		
		ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
		ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
		ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));

		Faint::Hardware::CPUInfo cpuInfo;
		Faint::Hardware::GPUInfo gpuInfo;

		/* CPU Information */
		static float cpuData[100] = {};
		static int cpuOffset = 0;

		cpuData[cpuOffset] = cpuInfo.CalculateCPULoad();
		cpuOffset = (cpuOffset + 1) % IM_ARRAYSIZE(cpuData);

		ImGui::Text("CPU Usage (%%)");
		ImGui::PlotLines("", cpuData, IM_ARRAYSIZE(cpuData), cpuOffset, nullptr, 0.0f, 100.0f, ImVec2(0, 60));

		/* GPU Information */
		static float gpuData[100] = {};
		static int gpuOffset = 0;

		gpuData[gpuOffset] = gpuInfo.CalculateGPULoad();
		gpuOffset = (gpuOffset + 1) % IM_ARRAYSIZE(gpuData);

		ImGui::Text("GPU Usage (%%)");
		ImGui::PlotLines("", gpuData, IM_ARRAYSIZE(gpuData), gpuOffset, nullptr, 0.0f, 100.0f, ImVec2(0, 60));
	}
	ImGui::End();
}
