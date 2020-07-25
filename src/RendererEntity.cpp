#include "RendererEntity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optick.h>

#include "Entity.hpp"
#include "GameState.hpp"
#include "Renderer.hpp"
#include "AssetManager.hpp"

const GLfloat vertices[] = {
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, -0.5f
};

const GLfloat uv_coords[] = {
    //Z+
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    //Z-
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    //X+
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    //X-
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    //Y+
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    //Y-
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
};

const GLuint magic = 993214;
GLuint Vbo = magic,Vao = magic,Vbo2 = magic;

GLuint RendererEntity::GetVao(){
    if (Vbo == magic) {
        glGenBuffers(1, &Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &Vbo2);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), uv_coords, GL_STATIC_DRAW);

        glGenVertexArrays(1, &Vao);
        glBindVertexArray(Vao);
        {
            glBindBuffer(GL_ARRAY_BUFFER, Vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, Vbo2);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);
        }
        glBindVertexArray(0);
    }
    return Vao;
}

RendererEntity::RendererEntity(unsigned int id)
{
    entityId = id;
}

RendererEntity::~RendererEntity() {
}

void RendererEntity::Render(RenderState& renderState, const World *world) {
	OPTICK_EVENT();
    glm::mat4 model = glm::mat4(1.0);
    const Entity &entity = world->GetEntity(entityId);
    model = glm::translate(model, entity.pos.glm());
    model = glm::translate(model, glm::vec3(0, entity.height / 2.0, 0));
    model = glm::scale(model, glm::vec3(entity.width, entity.height, entity.width));
    
	Shader *entityShader = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/entity")->shader.get();
	entityShader->SetUniform("model", model);
	entityShader->SetUniform("color", entity.renderColor);
    glCheckError();
    glDrawArrays(GL_LINES, 0, 24);

    glCheckError();
}
