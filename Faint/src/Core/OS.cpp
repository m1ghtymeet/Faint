#include "OS.h"
#include <filesystem>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef _WIN32
	#include <Windows.h>
	#include <shellapi.h>
	#include <ShlObj.h>
#else
	#include <cstdlib>
#endif

#include <Debug/Log.h>

void Moon::OS::CopyToClipboard(const std::string& value) {
	if (GLFWwindow* window = glfwGetCurrentContext ? glfwGetCurrentContext() : nullptr;
		window/*&& value.data()*/) {
		glfwSetClipboardString(NULL, value.c_str());
	}
}

std::string Moon::OS::GetFromClipboard() {
	if (GLFWwindow* window = glfwGetCurrentContext(); window) {
		const char* text = glfwGetClipboardString(window);
		return text ? std::string(text) : std::string();
	}
	return {};
}

bool Moon::OS::OpenIn(const std::string& filePath) {
#ifdef _WIN32
	HINSTANCE result = ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	return reinterpret_cast<intptr_t>(result) > 32;
#else
	// macOS: open, Linux: xdg-open
	std::string cmd =
	#ifdef __APPLE__
		"open \"" + filepath + "\"";
	#else
		"xdg-open \"" + filepath + "\"";
	#endif
	return system(cmd.c_str()) == 0;
#endif
}

bool Moon::OS::ShowInFileExplorer(const std::string& filePath) {
#ifdef _WIN32
	// This just opens the folder (not selecting the file)
	std::filesystem::path path(filePath);
	std::wstring wpath = path.parent_path().native();
	return reinterpret_cast<intptr_t>(
		ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOWNORMAL)
		) > 32;
	// OLD
	// ShellExecuteW(NULL, L"open", L"explorer.exe",
	// (L"/select,\"" + path.native() + L"\"").c_str(), NULL, SW_SHOWNORMAL)
	// )
#endif
}

std::string Moon::OS::GetExecutablePath() {
#ifdef _WIN32
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
	if (len == 0) return {};
	return std::filesystem::path(buffer).string();
#else
	// Linux/macOS fallback (less reliable)
	return std::filesystem::pread_symlink("/proc/self/exe").string(); // Linux only
#endif
}

std::string Moon::OS::GetExecutableDirectory() {
	return std::filesystem::path(GetExecutablePath()).parent_path().string();
}

//namespace Moon {
//	// dotnet publish -c Debug
//	std::string OS::CompileSln(const std::string& slnPath)
//	{
//		std::string output = "";
//		std::string err = "";
//		int result = Subprocess("dotnet build " + slnPath, output, err);
//
//		if (result != 0) {
//			HZ_CORE_ERROR("OS - Failed to execute 'dotnet build' command.");
//			HZ_CORE_ERROR(".NET - " + err);
//		}
//
//		return output;
//	}
//
//	int OS::Subprocess(const std::string& command, std::string& out, std::string& err)
//	{
//		auto splits = String::Split(command, ' ');
//		std::vector<const char*> command_line(splits.size() + 1);
//		for (int i = 0; i < splits.size(); i++)
//		{
//			command_line[i] = splits[i].c_str();
//		}
//
//		command_line.back() = nullptr;
//
//		struct subprocess_s subprocess;
//		char output[1024];
//		int result = subprocess_create(command_line.data(), subprocess_option_inherit_environment, &subprocess);
//		if (0 != result) {
//			// an error occurred!
//		}
//
//		int process_return;
//		result = subprocess_join(&subprocess, &process_return);
//		if (0 != result) {
//			// an error occurred!
//		}
//
//		FILE* p_stdout = subprocess_stdout(&subprocess);
//
//		std::string stdout_output;
//		fgets(output, 1024, p_stdout);
//		while (fgets(output, sizeof(output), p_stdout))
//		{
//			stdout_output += output;
//		}
//
//		FILE* p_stderr = subprocess_stderr(&subprocess);
//		std::string stderr_output;
//		char errOutput[1024];
//		while (fgets(errOutput, sizeof(errOutput), p_stderr))
//		{
//			stderr_output += errOutput;
//		}
//
//		out = stdout_output;
//		err = stderr_output;
//
//		return result;
//	}
//}
