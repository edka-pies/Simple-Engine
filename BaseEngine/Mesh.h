#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cfloat>
#include "Renderable.h"
#include "PhysicsUtils.h"
#include "Transform.h"
#include "ComponentRender.h"
#include "Texture.h"
#include "MemoryCheck.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoord;

	bool operator<(const Vertex& other) const
	{
		if (position.x != other.position.x) return position.x < other.position.x;
		if (position.y != other.position.y) return position.y < other.position.y;
		if (position.z != other.position.z) return position.z < other.position.z;

		if (textureCoord.x != other.textureCoord.x) return textureCoord.x < other.textureCoord.x;
		if (textureCoord.y != other.textureCoord.y) return textureCoord.y < other.textureCoord.y;

		if (normal.x != other.normal.x) return normal.x < other.normal.x;
		if (normal.y != other.normal.y) return normal.y < other.normal.y;
		if (normal.z != other.normal.z) return normal.z < other.normal.z;

		return false;
	}

	bool operator==(const Vertex& other) const
	{
		return position == other.position && normal == other.normal && textureCoord == other.textureCoord;
	}
};


struct ObjVertex
{
	uint32_t positionIndex;
	uint32_t uvIndex;
	uint32_t normalIndex;
};

struct Face
{

	Face()
	{
		vertexIndices.reserve(3);
		uvIndices.reserve(3);
		normalIndices.reserve(3);

	}
	std::vector<uint32_t> vertexIndices;
	std::vector<uint32_t> uvIndices;
	std::vector<uint32_t> normalIndices;

};

class Mesh : public ComponentRender
{
public:
	Mesh();

	~Mesh();

	virtual void Init() override;

	void SetVertexData(const std::vector<float>& vertexData);

	void SetVertexData(const std::vector<Vertex>& vertexData);

	void SetIndexData(const std::vector<unsigned int>& indicesData);

	void SetIndexData(const std::vector<Face> faceData);

	virtual void Render(Shader& shader, const glm::mat4& viewProjectionMatrix) override;

	virtual void CleanUp() override;

	// Texture API
	void SetTexture(std::shared_ptr<Texture> aTexture);
	std::shared_ptr<Texture> GetTexture();


	const std::vector<Vertex>& GetVertices() const { return vertices; }
	const std::vector<unsigned int>& GetIndices() const {return indices;}
	const AABB& GetLocalAABB() const { return localAABB; } 

	static std::shared_ptr<Mesh> CreateModelFromFile(const std::string& filePath);

	static AABB CalculateLocalAABB(const std::vector<Vertex>& vertices);

	AABB localAABB = { glm::vec3(0.0f), glm::vec3(0.0f) }; 
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

protected:
	unsigned int VBO; // Raw data on vertices
	unsigned int VAO;
	unsigned int EBO;
	unsigned int vertexCount = 0;
	unsigned int indexSize = 0;

	std::vector<float> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	// number of floats per vertex (3 for pos-only, 5 for pos+uv, 6 pos+normal, 8 pos+uv+normal)
	unsigned int vertexStride = 3;

	bool transformDirtyFlag = true;
	bool isCube = false;
	bool isHouse = false;

	std::shared_ptr<Texture> myTexture;
};
