#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Renderable.h"
#include "Transform.h"
#include "ComponentRender.h"
#include "Texture.h"
#include "MemoryCheck.h"

//#pragma pack(push,4)
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
//#pragma pack(pop)

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

	void SetRotation(const glm::vec3& newRotation);

	void SetPosition(const glm::vec3& newPosition);

	void SetScale(const glm::vec3& newScale);

	// Texture API
	void SetTexture(std::shared_ptr<Texture> aTexture);
	std::shared_ptr<Texture> GetTexture();

	inline  Transform& GetTransform() { return localTransform; }

	inline virtual const glm::mat4& GetModelMatrix() const { return localMatrix; }


public:
	static Mesh* MakeTriangle();

	static Mesh* MakeSquare();

	static Mesh* MakeCube();


	Transform localTransform;

	static std::shared_ptr<Mesh> CreateModelFromFile(const std::string& filePath);
private:



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

	glm::mat4 localMatrix = glm::mat4(1.0f);

	bool transformDirtyFlag = true;
	bool isCube = false;
	bool isHouse = false;

	std::shared_ptr<Texture> myTexture;
};
