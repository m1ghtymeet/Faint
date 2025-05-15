#include "hzpch.h"
#include "OS.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <string.h>
#include <tchar.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "String.h"
#include "Subprocess.hpp"

#include "FileSystem/Directory.h"

namespace Faint {

	// dotnet publish -c Debug

	void OS::CopyToClipboard(const std::string& value)
	{
		glfwSetClipboardString(NULL, value.c_str());
	}

	std::string OS::GetFromClipboard()
	{
		return std::string(glfwGetClipboardString(NULL));
	}

	void OS::OpenIn(const std::string& filePath)
	{
		// Windows
		ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	}

	int OS::RenameDirectory(const Ref<Directory>& dir, const std::string& newName)
	{
		std::string newDirPath = dir->Parent->FullPath + "/" + newName;

		std::error_code resultError;
		std::filesystem::rename(dir->FullPath.c_str(), newDirPath.c_str(), resultError);
		return resultError.value() == 0;
	}

	void OS::ShowInFileExplorer(const std::string& filePath)
	{
		ShellExecuteA(nullptr, "open", "explorer.exe", ("/select," + filePath).c_str(), nullptr, SW_SHOWDEFAULT);
	}

	std::string OS::CompileSln(const std::string& slnPath)
	{
		std::string output = "";
		std::string err = "";
		int result = Subprocess("dotnet build " + slnPath, output, err);

		if (result != 0) {
			HZ_CORE_ERROR("OS - Failed to execute 'dotnet build' command.");
			HZ_CORE_ERROR(".NET - {0}", err.c_str());
		}

		return output;
	}

	int OS::Subprocess(const std::string& command, std::string& out, std::string& err)
	{
		auto splits = String::Split(command, ' ');
		std::vector<const char*> command_line(splits.size() + 1);
		for (int i = 0; i < splits.size(); i++)
		{
			command_line[i] = splits[i].c_str();
		}

		command_line.back() = nullptr;

		struct subprocess_s subprocess;
		char output[1024];
		int result = subprocess_create(command_line.data(), subprocess_option_inherit_environment, &subprocess);
		if (0 != result) {
			// an error occurred!
		}

		int process_return;
		result = subprocess_join(&subprocess, &process_return);
		if (0 != result) {
			// an error occurred!
		}

		FILE* p_stdout = subprocess_stdout(&subprocess);

		std::string stdout_output;
		fgets(output, 1024, p_stdout);
		while (fgets(output, sizeof(output), p_stdout))
		{
			stdout_output += output;
		}

		FILE* p_stderr = subprocess_stderr(&subprocess);
		std::string stderr_output;
		char errOutput[1024];
		while (fgets(errOutput, sizeof(errOutput), p_stderr))
		{
			stderr_output += errOutput;
		}

		out = stdout_output;
		err = stderr_output;

		return result;
	}
}
