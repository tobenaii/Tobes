#pragma once
#include <string>
#include <map>
#include "Tobes/Core.h"
#include "Tobes/Common/Math/Vector3.h"
#include "Tobes/Common/Math/Matrix.h"

namespace Tobes
{
	class Shader
	{
		friend class Material;
		friend class MeshRenderer;
		friend class Application;
		friend class Renderer;

	private:
		unsigned int m_programID = -1;
		unsigned int m_shaderID;
		std::string m_source;

	private:
		Shader() {}
		void LoadVertexShader(const std::string& filePath);
		void LoadFragmentShader(const std::string& filePath);
		void ApplyShader();
		unsigned int GetProgramID();
		void LinkProgram();
		void SetUniformMat4(std::string name, Matrix mat);
		void SetUniform1f(std::string name, float value);
		void SetUniformVec3(std::string name, Vector3 vec);

		char const* GetShaderSource(const std::string& filePath);
		void CompileShader(char const*);
		void CreateProgram();
		void AttachShader();
		bool FindKey(std::string key);
		void AddKey(std::string key);

	private:
		std::map<std::string, unsigned int> m_uniformIds;
	};
}
