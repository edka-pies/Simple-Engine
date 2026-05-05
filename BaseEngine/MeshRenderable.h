#pragma once
#include <Glad/Glad/glad.h>
#include "Renderable.h"
#include "Shader.h"
#include <glm/glm/glm.hpp>
#include <iostream>

class MeshRenderable : public Renderable {
public:
    MeshRenderable(GLuint vao, GLsizei indexCount, const glm::mat4& model = glm::mat4(1.0f),
                   bool hasTexture = false, GLuint textureId = 0)
        : m_vao(vao), m_indexCount(indexCount), m_model(model), m_hasTexture(hasTexture), m_textureId(textureId) {}

    virtual ~MeshRenderable() {}

    virtual void Render(Shader& aShader, const glm::mat4& ) override {
        aShader.Use();

        aShader.SetMatrix(m_model, "modelMatrix");

        // Texture handling
        if (m_hasTexture && m_textureId != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureId);

            aShader.SetInt(0, "diffuseTexture");   
            aShader.SetInt(1, "useTexture");       
            aShader.SetFloat(1.0f, "textureMixFactor");
        } else {
            aShader.SetInt(0, "useTexture");       
        }

        // Bind VAO and draw 
        glBindVertexArray(m_vao);
        if (m_indexCount > 0) {
            glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        } else {
			std::cerr << "Warning: Attempting to draw MeshRenderable with zero indices. Skipping draw call.\n";
        }
        glBindVertexArray(0);

        if (m_hasTexture && m_textureId != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
            aShader.SetInt(0, "useTexture"); 
        }

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error after draw: 0x" << std::hex << err << std::dec << std::endl;
        }
    }

    virtual void Init() override {}
    virtual void CleanUp() override {}

    void SetModelMatrix(const glm::mat4& model) { m_model = model; }

private:
    GLuint m_vao;
    GLsizei m_indexCount;
    glm::mat4 m_model;
    bool m_hasTexture;
    GLuint m_textureId;
};