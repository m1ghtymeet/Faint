#include "Serializer.h"
#include <iostream>
#include <sstream>

//bool Moon::Helper::Serializer::Load(const std::string& filepath) {
//	std::ifstream file(filepath);
//	if (!file.is_open()) return false;
//
//	std::string line;
//	std::string currentKey;
//	std::stringstream blockContent;
//	int blockDepth = 0;
//
//	while (std::getline(file, line)) {
//		Trim(line);
//		if (line.empty()) continue;
//
//		if (blockDepth > 0) {
//			blockContent << line << '\n';
//			for (char c : line) {
//				if (c == '{') ++blockDepth;
//				else if (c == '}') --blockDepth;
//			}
//
//			if (blockDepth == 0 && !currentKey.empty()) {
//				values[currentKey] = blockContent.str();
//				currentKey.clear();
//				blockContent.str("");
//				blockContent.clear();
//			}
//			continue;
//		}
//
//		size_t equalPos = line.find('=');
//		if (equalPos == std::string::npos) continue;
//
//		std::string key = line.substr(0, equalPos);
//		std::string val = line.substr(equalPos + 1);
//		Trim(key);
//		Trim(val);
//
//		if (val == "{") {
//			currentKey = key;
//			blockDepth = 1;
//			blockContent.str("");
//			blockContent.clear();
//			blockContent << "{\n";
//		}
//		else {
//			// Check for string literal
//			if (!val.empty() && val.front() == '"' && val.back() == '"') {
//				val = val.substr(1, val.length() - 2);
//				values[key] = val;
//			}
//			else {
//				if (!val.empty() && val.front() == '"' && val.back() == '"') {
//					val = val.substr(1, val.length() - 2);
//					values[key] = val;
//				}
//				else {
//					try {
//						if (val.find('.') != std::string::npos)
//							values[key] = std::stof(val);
//						else
//							values[key] = std::stoi(val);
//					}
//					catch (...) {
//						values[key] = val; // fallback string
//					}
//				}
//			}
//		}
//	}
//
//	return true;
//}
//
//bool Moon::Helper::Serializer::LoadBinary(const std::string& p_filepath) {
//	std::ifstream in(p_filepath, std::ios::binary);
//	if (!in.is_open()) return false;
//
//
//	return true;
//}
//
//void Moon::Helper::Serializer::SaveBinary(const std::string& p_filepath) const
//{
//}
//
//std::optional<Moon::Helper::Value> Moon::Helper::Serializer::GetValue(const std::string& key) const {
//	auto it = values.find(key);
//	if (it != values.end())
//		return it->second;
//	return std::nullopt;
//}
//
//std::unordered_map<std::string, std::string> Moon::Helper::Serializer::ParseBlock(const std::string& blockContent) {
//	std::unordered_map<std::string, std::string> blockValues;
//	std::istringstream stream(blockContent);
//	std::string line;
//	std::string currentKey;
//	std::stringstream blockBuffer;
//	int blockDepth = 0;
//
//	while (std::getline(stream, line)) {
//		Trim(line);
//		if (line.empty()) continue;
//
//		if (blockDepth > 0) {
//			blockBuffer << line << '\n';
//			for (char c : line) {
//				if (c == '{') ++blockDepth;
//				else if (c == '}') --blockDepth;
//			}
//			if (blockDepth == 0 && !currentKey.empty()) {
//				blockValues[currentKey] = blockBuffer.str();
//				currentKey.clear();
//				blockBuffer.str("");
//				blockBuffer.clear();
//			}
//			continue;
//		}
//
//		size_t equalPos = line.find('=');
//		if (equalPos == std::string::npos) continue;
//
//		std::string key = line.substr(0, equalPos);
//		std::string val = line.substr(equalPos + 1);
//		Trim(key);
//		Trim(val);
//
//		if (val == "{") {
//			currentKey = key;
//			blockDepth = 1;
//			blockBuffer.str("");
//			blockBuffer.clear();
//			blockBuffer << "{\n";
//		}
//		else {
//			// Remove quotes
//			if (!val.empty() && val.front() == '"' && val.back() == '"')
//				val = val.substr(1, val.length() - 2);
//			blockValues[key] = val;
//		}
//	}
//
//	return blockValues;
//}
//
//std::vector<std::unordered_map<std::string, std::string>> Moon::Helper::Serializer::ParseBlockArray(const std::string& blockStr) {
//	std::vector<std::unordered_map<std::string, std::string>> result;
//	std::istringstream stream(blockStr);
//	std::string line;
//	std::stringstream currentBlock;
//	float blockDepth = 0;
//	bool inBlock = false;
//
//	while (std::getline(stream, line)) {
//		Trim(line);
//		if (line.empty()) continue;
//
//		for (char c : line) {
//			if (c == '{') {
//				if (blockDepth == 1) {
//					currentBlock.str("");
//					currentBlock.clear();
//					inBlock = true;
//				}
//				blockDepth++;
//			}
//			if (inBlock) {
//				currentBlock << line << "\n";
//			}
//			if (c == '}') {
//				blockDepth--;
//				if (blockDepth == 1 && inBlock) {
//					auto parsed = ParseBlock(currentBlock.str());
//					if (!parsed.empty())
//						result.push_back(parsed);
//					inBlock = false;
//				}
//			}
//		}
//	}
//
//	return result;
//}
//
//std::vector<float> Moon::Helper::Serializer::ParseFloatArray(const std::string& data) {
//	std::vector<float> result;
//	std::stringstream ss(data);
//	std::string item;
//	while (std::getline(ss, item, ';')) {
//		try {
//			if (!item.empty())
//				result.push_back(std::stof(item));
//		}
//		catch (...) {
//			// skip invalid floats
//		}
//	}
//	return result;
//}
//
//bool Moon::Helper::Serializer::ConvertOBJToCBA(const std::string& objPath, const std::string& cbaPath) {
//	std::ifstream file(objPath);
//	if (!file.is_open()) {
//		std::cerr << "Failed to open OBJ file.\n";
//		return false;
//	}
//
//	std::vector<glm::vec3> vertices;
//	std::unordered_map<std::string, Section> sections;
//
//	std::string currentSection = "default";
//	int vertexOffset = 0;
//
//	std::string line;
//	while (std::getline(file, line)) {
//		std::istringstream ss(line);
//		std::string prefix;
//		ss >> prefix;
//
//		if (prefix == "o" || prefix == "g") {
//			ss >> currentSection;
//			if (sections.find(currentSection) == sections.end())
//				sections[currentSection] = Section{ currentSection };
//		}
//		else if (prefix == "v") {
//			glm::vec3 v;
//			ss >> v.x >> v.y >> v.z;
//			vertices.push_back(v);
//		}
//		else if (prefix == "f") {
//			std::vector<int> faceIndices;
//			std::string vert;
//			while (ss >> vert) {
//				size_t pos = vert.find('/');
//				int idx = std::stoi(vert.substr(0, pos)) - 1;
//				faceIndices.push_back(idx);
//			}
//
//			for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
//				sections[currentSection].indices.push_back(faceIndices[0]);
//				sections[currentSection].indices.push_back(faceIndices[i]);
//				sections[currentSection].indices.push_back(faceIndices[i + 1]);
//			}
//		}
//	}
//	if (vertices.empty()) {
//		std::cerr << "No vertices in OBJ!\n";
//		return false;
//	}
//
//	// Flatten vertices for each section
//	for (auto& [name, section] : sections) {
//		std::unordered_map<int, int> remap;
//		std::vector<float> newVertices;
//		int newIndex = 0;
//		for (int idx : section.indices) {
//			if (remap.find(idx) == remap.end()) {
//				remap[idx] = newIndex++;
//				glm::vec3& v = vertices[idx];
//				newVertices.push_back(v.x);
//				newVertices.push_back(v.y);
//				newVertices.push_back(v.z);
//			}
//		}
//
//		for (int& idx : section.indices)
//			idx = remap[idx];
//
//		section.vertices = std::move(newVertices);
//	}
//
//	file.close();
//
//	// Generate :).cba content
//	std::ofstream out(cbaPath);
//	if (!out.is_open()) return false;
//
//	out << "mesh = {\n";
//	int totalIndices = 0;
//	int totalVertices = 0;
//
//	for (const auto& [name, section] : sections) {
//		totalVertices += section.vertices.size();
//		totalIndices += section.indices.size();
//	}
//
//	out << "\tvertexCount = " << totalVertices / 3 << '\n';
//	out << "\tpolyCount = " << totalIndices / 3 << '\n';;
//
//	out << "\tindices = ";
//	for (const auto& [name, section] : sections) {
//		WriteArray<int>(out, section.indices);
//	}
//	out << "\n";
//
//	out << "	sections = {\n";
//	for (const auto& [name, section] : sections) {
//		out << "\t\t{\n";
//		out << "\t\t\tid = " << name << "\n";
//		out << "\t\t\tvertices = ";
//		for (size_t i = 0; i < section.vertices.size(); i++) {
//			out << section.vertices[i];
//			if (i < section.vertices.size() - 1) out << ';';
//		}
//		out << "\n\t\t}\n";
//	}
//	out << "\t}\n";
//	out << "}\n";
//	out.close();
//
//	std::cout << "Converted OBJ to CBA successfully.\n";
//	return true;
//}
//
//void Moon::Helper::Serializer::PrintValues() const {
//	for (const auto& [key, val] : values) {
//		std::cout << key << " = ";
//		if (std::holds_alternative<int>(val))
//			std::cout << std::get<int>(val);
//		else if (std::holds_alternative<float>(val))
//			std::cout << std::get<float>(val);
//		else if (std::holds_alternative<std::string>(val)) {
//			std::string str = std::get<std::string>(val);
//			if (str.find('\n') != std::string::npos || str.find('{') != std::string::npos) {
//				std::cout << "[BLOCK with " << std::count(str.begin(), str.end(), '\n') << " lines]\n";
//				std::cout << str << '\n';
//			}
//			else
//				std::cout << '"' << str << '"';
//		}
//		std::cout << '\n';
//	}
//}
//