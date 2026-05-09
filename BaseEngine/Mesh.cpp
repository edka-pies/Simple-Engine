#include <Glad/Glad/glad.h>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Loader.h"
#include "PhysicsUtils.h"
#include <GLFW/Include/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <map>
#include <array>
#include <memory>

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), myTexture(nullptr), vertexStride(3)
{
}

Mesh::~Mesh()
{
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (EBO != 0) glDeleteBuffers(1, &EBO);
}


void Mesh::Init()
{
	// Abort if we have neither primitives nor model data
	if (vertexBuffer.empty() && vertices.empty()) return;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// --- SCENARIO A: WE ARE LOADING A 3D MODEL (Uses Vertex Struct) ---
	if (!vertices.empty())
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		// Layout 0: Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		// Layout 1: UVs (Swapped to match your shader!)
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoord));

		// Layout 2: Normal (Swapped to match your shader!)
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// Layout 3: Bone IDs (Notice the 'I' in glVertexAttribIPointer for Integers!)
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

		// Layout 4: Bone Weights
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
	}
	// --- SCENARIO B: WE ARE LOADING A PRIMITIVE (Uses Float Array) ---
	else
	{
		glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);

		if (vertexStride == 5) { // Cube
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		else if (vertexStride == 8) { // Standard Object without bones
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}
		else { // Basic fallback
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}
	}

	// --- INDICES (Stays exactly the same) ---
	if (indexSize > 0)
	{
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), indexBuffer.data(), GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::SetVertexData(const std::vector<float>& vertexData)
{
	vertexBuffer = vertexData;

	if (isCube)
	{
		vertexStride = 5; // pos + uv
	}
	else
	{
		if (vertexBuffer.size() % 8 == 0)
			vertexStride = 8;
		else if (vertexBuffer.size() % 6 == 0)
			vertexStride = 6;
		else if (vertexBuffer.size() % 5 == 0)
			vertexStride = 5;
		else
			vertexStride = 3;
	}

	if (vertexStride == 0) vertexStride = 3;
	vertexCount = static_cast<unsigned int>(vertexBuffer.size() / vertexStride);
}

void Mesh::SetVertexData(const std::vector<Vertex>& vertexData)
{
	// Clear the float buffer because we are using the Struct instead!
	vertexBuffer.clear();

	// Save the raw structs
	vertices = vertexData;
	vertexCount = static_cast<unsigned int>(vertices.size());
}

void Mesh::SetIndexData(const std::vector<unsigned int>& indexData)
{
	indexBuffer = indexData;
	indexSize = static_cast<unsigned int>(indexBuffer.size());
}

void Mesh::SetIndexData(const std::vector<Face> faceData)
{
	for (const Face& faces : faceData)
	{
		for (auto idx : faces.vertexIndices)
		{
			indexBuffer.push_back(idx);
		}
	}

	indexSize = static_cast<unsigned int>(indexBuffer.size());
}

void Mesh::Render(Shader& shader, const glm::mat4& viewProjectionMatrix)
{
	shader.SetMatrix(viewProjectionMatrix, "modelMatrix");

	// Texture: bind to unit 0
	if (myTexture != nullptr && myTexture->textureObject != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, myTexture->textureObject);
		shader.SetInt(0, "diffuseTexture"); 
		shader.SetInt(1, "useTexture");   
	}
	else
	{
		shader.SetInt(0, "useTexture");    
	}

	glBindVertexArray(VAO);

	if (EBO == 0)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);

	// Unbind texture
	if (myTexture != nullptr && myTexture->textureObject != 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		shader.SetInt(0, "useTexture");
	}
}

void Mesh::CleanUp()
{
	if (VAO != 0) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
	if (VBO != 0) { glDeleteBuffers(1, &VBO); VBO = 0; }
	if (EBO != 0) { glDeleteBuffers(1, &EBO); EBO = 0; }
}

std::shared_ptr<Mesh> Mesh::CreateModelFromFile(const std::string& filePath)
{
	Loader loader{};
	loader.LoadModel(filePath);
	std::shared_ptr<Mesh> resultMesh = std::make_shared<Mesh>();
	resultMesh->SetVertexData(loader.vertices);
	resultMesh->vertices = loader.vertices;
	resultMesh->SetIndexData(loader.indices);
	resultMesh->indices = loader.indices;
	resultMesh->localAABB = CalculateLocalAABB(loader.vertices);
	return resultMesh;
}

// Texture API implementation
void Mesh::SetTexture(std::shared_ptr<Texture> aTexture)
{
	myTexture = aTexture;
}

std::shared_ptr<Texture> Mesh::GetTexture() 
{
	return myTexture;
}