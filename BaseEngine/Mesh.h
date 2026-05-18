#pragma once
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <glm/glm/glm.hpp>
#include "Texture.h"
#include "PhysicsUtils.h" // For AABB

#define MAX_BONE_INFLUENCE 4

struct BoneInfo {
	int id;               // The index we will send to the GPU
	glm::mat4 offset;     // The Inverse Bind Matrix from Assimp
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoord;

	int m_BoneIDs[MAX_BONE_INFLUENCE]; // Which bones affect this vertex?
	float m_Weights[MAX_BONE_INFLUENCE]; // How much does each bone affect it?

	// Constructor to initialize empty weights
	Vertex() {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			m_BoneIDs[i] = -1; // -1 means "No bone assigned"
			m_Weights[i] = 0.0f;
		}
	}

	Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 uv)
		: position(p), normal(n), textureCoord(uv)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			m_BoneIDs[i] = -1;
			m_Weights[i] = 0.0f;
		}
	}

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

class Mesh
{
public:
	Mesh();
	~Mesh();

	void Init();
	void CleanUp();

	// Setting Data
	void SetVertexData(const std::vector<float>& vertexData);
	void SetVertexData(const std::vector<Vertex>& vertexData);
	void SetIndexData(const std::vector<unsigned int>& indicesData);
	void SetIndexData(const std::vector<Face> faceData);

	// --- REFACTORED: Renamed to Draw, removed the Matrix! ---
	void Draw(class Shader& shader);

	// Texture API
	void SetTexture(std::shared_ptr<Texture> aTexture);
	std::shared_ptr<Texture> GetTexture();

	const std::vector<Vertex>& GetVertices() const { return vertices; }
	const std::vector<unsigned int>& GetIndices() const { return indices; }
	const AABB& GetLocalAABB() const { return localAABB; }

	static std::shared_ptr<Mesh> CreateModelFromFile(const std::string& filePath);
	static AABB CalculateLocalAABB(const std::vector<Vertex>& vertices);

	AABB localAABB = { glm::vec3(0.0f), glm::vec3(0.0f) };
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

protected:
	unsigned int VBO, VAO, EBO;
	unsigned int vertexCount = 0;
	unsigned int indexSize = 0;
	unsigned int vertexStride = 3;

	std::vector<float> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	std::map<std::string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;

	std::shared_ptr<Texture> myTexture;
};
