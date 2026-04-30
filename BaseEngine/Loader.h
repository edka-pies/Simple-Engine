#pragma once
#include <vector>
#include "Mesh.h"

class Loader
{
public:
	Loader() = default;
	~Loader() = default;
	void LoadModel(const std::string& filePath);
	void ProcessData(std::vector<glm::vec3> positions, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals);
	void Triangulate(Face& face);
	std::vector<Vertex> vertices{};
	std::vector<Face> faces{};
	//Stores all indexes
	std::vector<uint32_t> indices{};

private:
	enum class LoadState
	{
		OpenFile,
		Vertices,
		UVs,
		Faces,
		PrepForBuffering,
		Done
	} type;
};

