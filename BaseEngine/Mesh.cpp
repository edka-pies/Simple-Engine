#include <Glad/Glad/glad.h>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Loader.h"
#include <GLFW/Include/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <map>
#include <array>

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
	if (vertexBuffer.empty())
	{
		return;
	}
	else
	{
		std::cout << "Vertex floats: " << vertexBuffer.size() << " stride: " << vertexStride << " vertices: " << vertexCount << std::endl;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);

	if (indexSize > 0)
	{
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), indexBuffer.data(), GL_STATIC_DRAW);
		std::cout << "Index count: " << indexSize << std::endl;
	}

	if (vertexStride == 5)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	else if (vertexStride == 8)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	else if (vertexStride == 6)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	else
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
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
	vertexBuffer.clear();
	vertexBuffer.reserve(vertexData.size() * 8); 

	for (const Vertex& vertices : vertexData)
	{
		vertexBuffer.push_back(vertices.position.x);
		vertexBuffer.push_back(vertices.position.y);
		vertexBuffer.push_back(vertices.position.z);

		vertexBuffer.push_back(vertices.textureCoord.x);
		vertexBuffer.push_back(vertices.textureCoord.y);

		vertexBuffer.push_back(vertices.normal.x);
		vertexBuffer.push_back(vertices.normal.y);
		vertexBuffer.push_back(vertices.normal.z);
	}

	vertexStride = 8;
	vertexCount = static_cast<unsigned int>(vertexBuffer.size() / vertexStride);
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
	if (transformDirtyFlag)
	{
		localMatrix = localTransform.mat4();
		transformDirtyFlag = false;
	}

	// upload per-object model matrix (vertex shader expects modelMatrix; view/proj provided by renderer)
	shader.SetMatrix(localMatrix, "modelMatrix");

	// Texture: bind to unit 0 and tell shader to use it
	if (myTexture != nullptr && myTexture->textureObject != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, myTexture->textureObject);
		shader.SetInt(0, "diffuseTexture"); // sampler uses texture unit 0
		shader.SetInt(1, "useTexture");     // enable texture sampling
	}
	else
	{
		shader.SetInt(0, "useTexture");     // disable texture sampling
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

	// Unbind texture and restore shader uniform state
	if (myTexture != nullptr && myTexture->textureObject != 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		shader.SetInt(0, "useTexture"); // disable for following draws
	}
}

void Mesh::CleanUp()
{
	if (VAO != 0) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
	if (VBO != 0) { glDeleteBuffers(1, &VBO); VBO = 0; }
	if (EBO != 0) { glDeleteBuffers(1, &EBO); EBO = 0; }
}

void Mesh::SetRotation(const glm::vec3& newRotation)
{
	localTransform.rotation = newRotation;
	transformDirtyFlag = true;
}

void Mesh::SetPosition(const glm::vec3& newPosition)
{
	localTransform.position = newPosition;
	transformDirtyFlag = true;
}

void Mesh::SetScale(const glm::vec3& newScale)
{
	localTransform.scale = newScale;
	transformDirtyFlag = true;
}

Mesh* Mesh::MakeTriangle()
{
	Mesh* mesh = new Mesh();

	mesh->SetVertexData(
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f , 0.0f,
			0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f
		}
	);

	return mesh;
}

Mesh* Mesh::MakeSquare()
{
	Mesh* mesh = new Mesh();

	mesh->SetVertexData(
		{
			 0.5f,  0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		}
	);

	mesh->SetIndexData(
		{
			0, 1, 3,
			1, 2, 3
		}
	);

	return mesh;
}

Mesh* Mesh::MakeCube()
{
	Mesh* mesh = new Mesh();

	mesh->isCube = true;

	mesh->SetVertexData(
		{
			// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

		// Back face (Z-)
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		 // Bottom face (Y-)
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		 // Top face (Y+)
		 -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 0.0f,

		  // Left face (X-)
		  -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		  -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		  -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

		  // Right face (X+)
		   0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		   0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		   0.5f, -0.5f,  0.5f,  1.0f, 0.0f
		}
	);

	mesh->SetIndexData(
		{
			// Front face
		0, 2, 1, 2, 0, 3,
		// Back face
		4, 6, 5, 6, 4, 7,
		// Bottom face
		8, 10, 9, 10, 8, 11,
		// Top face
		12, 14, 13, 14, 12, 15,
		// Left face
		16, 18, 17, 18, 16, 19,
		// Right face
		20, 22, 21, 22, 20, 23
		}
	);

	return mesh;
}

std::shared_ptr<Mesh> Mesh::CreateModelFromFile(const std::string& filePath)
{
	Loader loader{};
	loader.LoadModel(filePath);
	std::shared_ptr<Mesh> resultMesh = std::make_shared<Mesh>();
	resultMesh->SetVertexData(loader.vertices);
	resultMesh->SetIndexData(loader.indices);
	return resultMesh;
}

// Texture API implementation
void Mesh::SetTexture(std::shared_ptr<Texture> aTexture)
{
	myTexture = aTexture;
	// no immediate GL calls here; texture is bound at render time
}

std::shared_ptr<Texture> Mesh::GetTexture() 
{
	return myTexture;
}
