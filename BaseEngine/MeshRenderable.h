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

    virtual void Render(Shader& aShader, const glm::mat4& /*viewProjectionMatrix*/) override {
        // Ensure shader is bound (ForwardRendererPass already calls Use(), but this is safe)
        aShader.Use();

        // Upload the per-object model matrix expected by the vertex shader
        aShader.SetMatrix(m_model, "modelMatrix");

        // Texture handling: bind texture to unit 0 and tell the shader which unit to sample.
        if (m_hasTexture && m_textureId != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureId);

            // Set sampler unit and enable sampling.
            aShader.SetInt(0, "diffuseTexture");    // sampler = texture unit 0
            aShader.SetInt(1, "useTexture");        // useTexture = 1
            aShader.SetFloat(1.0f, "textureMixFactor");
        } else {
            // Disable texture sampling for this object
            aShader.SetInt(0, "useTexture");        // useTexture = 0
        }

        // Bind VAO and draw (assumes VAO configured with attribute indices 0=pos,1=uv,2=normal)
        glBindVertexArray(m_vao);
        if (m_indexCount > 0) {
            glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        } else {
            // If using non-indexed geometry, you may set a vertex count member and call glDrawArrays
            // glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
        }
        glBindVertexArray(0);

        // Unbind texture and restore shader uniform state
        if (m_hasTexture && m_textureId != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
            aShader.SetInt(0, "useTexture"); // set useTexture = 0
        }

        // Debug: surface any GL errors (remove in production)
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error after draw: 0x" << std::hex << err << std::dec << std::endl;
        }
    }

    virtual void Init() override {}
    virtual void CleanUp() override {}

    // helper to set model
    void SetModelMatrix(const glm::mat4& model) { m_model = model; }

private:
    GLuint m_vao;
    GLsizei m_indexCount; // number of indices; 0 if not using indexed draw
    glm::mat4 m_model;
    bool m_hasTexture;
    GLuint m_textureId;
};