#pragma once
#include <string>

class File;

class ContentManager
{
public:
	void LoadFile(std::string filePath);
	void SaveFile();

private:
	File* m_file;
	const std::string m_supportedImageTypes[3]{".png", ".jpg", ".tga"};
	const std::string m_supportedModelTypes[2]{".obj", ".fbx"};
	std::string m_fileName;
};