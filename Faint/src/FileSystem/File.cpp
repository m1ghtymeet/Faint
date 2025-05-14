#include "File.h"
#include "FileSystem.h"
#include "../Core/String.h"

namespace Faint {

	File::File(Ref<Directory> parentDir, const std::string& absolutePath, const std::string& name, const std::string& type)
		: name(name),
		type(type),
		absolutePath(absolutePath),
		relativePath(FileSystem::AbsoluteToRelative(absolutePath)),
		modified(false)
	{
	}

	FileType File::GetFileType() const
	{
		std::string ext = GetExtension();
		if (ext == ".png" || ext == ".jpg" || ext == ".dds") {
			return FileType::Image;
		}
		if (ext == ".material") {
			return FileType::Material;
		}
		if (ext == ".scene") {
			return FileType::Scene;
		}
		if (ext == ".project") {
			return FileType::Project;
		}
		if (ext == ".prefab") {
			return FileType::Prefab;
		}
		if (ext == ".dll") {
			return FileType::Assembly;
		}
		if (ext == ".cs")
		{
			return FileType::NetScript;
		}
		if (ext == ".wren")
		{
			return FileType::Script;
		}
		//if (ext == ".sln")
		//	return FileType::;
		if (ext == ".mesh")
		{
			return FileType::Mesh;
		}
		if (ext == ".obj" || ext == ".fbx")
		{
			return FileType::MeshAsset;
		}
		if (ext == ".wav" || ext == ".mp3")
		{
			return FileType::Audio;
		}

		return FileType::Unknown;
	}

	std::string File::GetFileTypeAsString() const
	{
		std::string ext = GetExtension();
		if (ext == ".scene")
		{
			return "Scene";
		}
		if (ext == ".project")
		{
			return "Project";
		}
		if (ext == ".cs")
		{
			return "C#";
		}
		if (ext == ".wren")
		{
			return "Script";
		}
		if (ext == ".material")
		{
			return "Material";
		}
		if (ext == ".mesh" || ext == ".obj")
		{
			return "Model";
		}
		if (ext == ".png" || ext == ".jpg" || ext == ".dds")
		{
			return "Texture";
		}

		return "Unknown File";
	}
}
