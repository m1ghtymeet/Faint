#include "Core/Application.h"
#include <Analytics/GPUInfo.h>
#include "Core/ProjectWindow.h"

#undef APIENTRY
#include <Windows.h>

//FORCE_DEDICATED_GPU

bool TryRun(const std::filesystem::path& projectPath) {

	std::unique_ptr<Application> app;

	try {
		app = std::make_unique<Application>(projectPath);
	}
	catch(...) {}

	if (app) {
		app->Run();
		return true;
	}

	return false;
}

int main(int argc, char** argv) {
#ifdef FT_DEBUG
	// Enable VT processing on Windows 10 cmd
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return -1;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode)) return -1;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#endif

	std::optional<std::filesystem::path> projectPath{ "" };

	if (argc < 2) {
		ProjectWindow project;
	
		if (auto result = project.Run()) {
			projectPath = result;
		}
	}
	else {
		projectPath = argv[1];
	}

	if (projectPath && !projectPath->empty()) {
		if (!TryRun(projectPath.value())) {
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

#ifndef FT_DEBUG
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	main(__argc, __argv);
}
#endif