#include "RendererEntity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <optick.h>

#include "Entity.hpp"
#include "World.hpp"


RendererEntity::RendererEntity(unsigned int id): entityId(id) {}

void RendererEntity::Render(std::shared_ptr<Gal::Pipeline> pipeline, const World *world) {
    OPTICK_EVENT();
    glm::mat4 model = glm::mat4(1.0);
    const Entity &entity = world->GetEntity(entityId);
    model = glm::translate(model, entity.pos.glm());
    model = glm::translate(model, glm::vec3(0, entity.height / 2.0, 0));
    model = glm::scale(model, glm::vec3(entity.width, entity.height, entity.width));
    
    pipeline->SetShaderParameter("model", model);
    pipeline->SetShaderParameter("color", entity.renderColor);
}
