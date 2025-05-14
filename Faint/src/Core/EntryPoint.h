#pragma once
#include "Core/Base.h"
#include "Core/Application.h"
#include "Analytics/GPUInfo.h"
#include "Analytics/RAMInfo.h"
#include "FileSystem/FileSystem.h"

#ifdef FT_PLATFORM_WINDOWS

extern Faint::Application* Faint::CreateApplication();

int main(int argc, char** argv) {

	Faint::Log::Init();
	Hardware::GPUInfo gpuInfo;
	
	std::vector<std::string> args;
	for (uint32_t i = 0; i < argc; i++) args.push_back(std::string(argv[i]));
	//Faint::FileSystem::SetRootDirectory(Faint::FileSystem::GetParentPath(args[0]) + "assets");

#ifdef FT_DEBUG
	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
#endif
	auto app = Faint::CreateApplication();
#ifdef FT_DEBUG
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
#endif
	app->Run();
#ifdef FT_DEBUG
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
#endif
	delete app;
#ifdef FT_DEBUG
	HZ_PROFILE_END_SESSION();
#endif
}

#endif