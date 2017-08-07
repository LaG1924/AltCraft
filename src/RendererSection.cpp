#include "RendererSection.hpp"

#include <thread>

const GLfloat vertices[] = {
		0, 0, 0,
		1, 0, 1,
		1, 0, 0,

		0, 0, 0,
		0, 0, 1,
		1, 0, 1,
};

const GLfloat uv_coords[] = {
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
};

const GLuint magicUniqueConstant = 88375;
GLuint RendererSection::VboVertices = magicUniqueConstant;
GLuint RendererSection::VboUvs = magicUniqueConstant;
std::map<GLuint, int> RendererSection::refCounterVbo;
std::map<GLuint, int> RendererSection::refCounterVao;


RendererSection::RendererSection(World *world, Vector position) : sectionPosition(position), world(world) {
	if (VboVertices == magicUniqueConstant) {
		glGenBuffers(1, &VboVertices);
		glGenBuffers(1, &VboUvs);

		//Cube vertices
		glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		//Cube UVs
		glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), uv_coords, GL_STATIC_DRAW);

		LOG(INFO) << "Created VBOs with vertices (" << VboVertices << ") and UVs (" << VboUvs
		          << ") for ordinary blocks";
	}

	glGenBuffers(1, &VboTextures);
	if (refCounterVbo.find(VboTextures) == refCounterVbo.end())
		refCounterVbo[VboTextures] = 0;
	refCounterVbo[VboTextures]++;

	glGenBuffers(1, &VboModels);
	if (refCounterVbo.find(VboModels) == refCounterVbo.end())
		refCounterVbo[VboModels] = 0;
	refCounterVbo[VboModels]++;

	glGenBuffers(1, &VboColors);
	if (refCounterVbo.find(VboColors) == refCounterVbo.end())
		refCounterVbo[VboColors] = 0;
	refCounterVbo[VboColors]++;

	glGenVertexArrays(1, &Vao);
	if (refCounterVao.find(Vao) == refCounterVao.end())
		refCounterVao[Vao] = 0;
	refCounterVao[Vao]++;

	glBindVertexArray(Vao);
	{
		//Cube vertices
		GLuint VertAttribPos = 0;
		glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
		glVertexAttribPointer(VertAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(VertAttribPos);

		//Cube UVs
		GLuint UvAttribPos = 2;
		glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
		glVertexAttribPointer(UvAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(UvAttribPos);

		//Textures
		GLuint textureAttribPos = 7;
		glBindBuffer(GL_ARRAY_BUFFER, VboTextures);
		glVertexAttribPointer(textureAttribPos, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(textureAttribPos);
		glVertexAttribDivisor(textureAttribPos, 1);
		glCheckError();

		//Blocks models
		GLuint matAttribPos = 8;
		size_t sizeOfMat4 = 4 * 4 * sizeof(GLfloat);
		glBindBuffer(GL_ARRAY_BUFFER, VboModels);
		glVertexAttribPointer(matAttribPos + 0, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, nullptr);
		glVertexAttribPointer(matAttribPos + 1, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (1 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(matAttribPos + 2, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (2 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(matAttribPos + 3, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (3 * 4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(matAttribPos + 0);
		glEnableVertexAttribArray(matAttribPos + 1);
		glEnableVertexAttribArray(matAttribPos + 2);
		glEnableVertexAttribArray(matAttribPos + 3);
		glVertexAttribDivisor(matAttribPos + 0, 1);
		glVertexAttribDivisor(matAttribPos + 1, 1);
		glVertexAttribDivisor(matAttribPos + 2, 1);
		glVertexAttribDivisor(matAttribPos + 3, 1);

		//Color
		GLuint colorAttribPos = 12;
		glBindBuffer(GL_ARRAY_BUFFER, VboColors);
		glVertexAttribPointer(colorAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(colorAttribPos);
		glVertexAttribDivisor(colorAttribPos, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
	glCheckError();
}

RendererSection::~RendererSection() {
	refCounterVbo[VboTextures]--;
	refCounterVbo[VboModels]--;
	refCounterVbo[VboColors]--;
	refCounterVao[Vao]--;
	if (refCounterVbo[VboTextures] <= 0)
		glDeleteBuffers(1, &VboTextures);

	if (refCounterVbo[VboModels] <= 0)
		glDeleteBuffers(1, &VboTextures);
	if (refCounterVbo[VboColors] <= 0)
		glDeleteBuffers(1, &VboColors);

	if (refCounterVao[Vao] <= 0)
		glDeleteVertexArrays(1, &Vao);
}

void RendererSection::Render(RenderState &renderState) {
	if (!isEnabled) return;
	if (!models.empty()) {
		PrepareRender();
	}
	renderState.SetActiveVao(Vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numOfFaces);
	glCheckError();
}

Section *RendererSection::GetSection() {
	return &world->GetSection(sectionPosition);
}

RendererSection::RendererSection(const RendererSection &other) {
	this->world = other.world;
	this->VboModels = other.VboModels;
	this->VboTextures = other.VboTextures;
	this->VboColors = other.VboColors;
	this->sectionPosition = other.sectionPosition;
	this->Vao = other.Vao;
	this->numOfFaces = other.numOfFaces;
	this->models = other.models;
	this->textures = other.textures;
	this->colors = other.colors;
	this->hash = other.hash;

	refCounterVbo[VboTextures]++;
	refCounterVbo[VboModels]++;
	refCounterVbo[VboColors]++;
	refCounterVao[Vao]++;
}

void RendererSection::SetEnabled(bool isEnabled) {
	this->isEnabled = isEnabled;
}

bool RendererSection::IsNeedResourcesPrepare() {
	size_t currentHash = world->GetSection(sectionPosition).GetHash();
	bool isNeedUpdate = currentHash != hash;
	return isNeedUpdate;
}

void RendererSection::PrepareResources() {
	const std::map<BlockTextureId,glm::vec4> &textureAtlas = AssetManager::Instance().GetTextureAtlasIndexes();
	Section &section = world->GetSection(sectionPosition);
	models.clear();
	textures.clear();
	colors.clear();
	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				Vector blockPos = Vector(x, y, z) + (sectionPosition * 16);
				Block block = world->GetBlock(blockPos);
				if (block.id == 0)
					continue;

				auto checkBlockVisibility = [&](Vector block) -> bool {
					return section.GetBlock(block).id == 0 ||
					       section.GetBlock(block).id == 31 ||
					       section.GetBlock(block).id == 18;
				};

				unsigned char isVisible = 0;
				if (x == 0 || x == 15 || y == 0 || y == 15 || z == 0 || z == 15) {
					isVisible = 0b1111'1111; //All faces is visible
				} else {
					isVisible |= checkBlockVisibility(Vector(x - 1, y, z)) << 0;
					isVisible |= checkBlockVisibility(Vector(x + 1, y, z)) << 1;
					isVisible |= checkBlockVisibility(Vector(x, y + 1, z)) << 2;
					isVisible |= checkBlockVisibility(Vector(x, y - 1, z)) << 3;
					isVisible |= checkBlockVisibility(Vector(x, y, z - 1)) << 4;
					isVisible |= checkBlockVisibility(Vector(x, y, z + 1)) << 5;
				}

				if (isVisible == 0x00)
					continue;

				glm::mat4 transform;
				transform = glm::translate(transform, glm::vec3(sectionPosition.GetX() * 16,
				                                                sectionPosition.GetY() * 16,
				                                                sectionPosition.GetZ() * 16));
				transform = glm::translate(transform, glm::vec3(x, y, z));
				glm::vec3 biomeColor(0.275, 0.63, 0.1);
				glm::vec3 color(0.0f, 0.0f, 0.0f);
				if (block.id == 31 || block.id == 18)
					color = biomeColor;

				if (block.id == 31) { //X-cross like blocks rendering
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 2));
					for (int i = 0; i < 4; i++) {
						textures.push_back(texture->second);
						colors.push_back(color);
					}
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0.15f, 0, 0.15f));
					faceTransform = glm::scale(faceTransform, glm::vec3(1.0f, 0.9f, 1.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0));
					for (int i = 0; i < 4; i++) {
						models.push_back(faceTransform);
						faceTransform = glm::translate(faceTransform, glm::vec3(0.0f, 0.0f, 0.5f));
						faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
						faceTransform = glm::translate(faceTransform, glm::vec3(0.0f, 0.0f, -0.5f));
					}
					continue;
				}

				if (isVisible >> 0 & 0x1) { //east side of block (X+)
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 2));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					colors.push_back(color);
				}
				if (isVisible >> 1 & 0x1) { //west side X-
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 3));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					colors.push_back(color);
				}
				if (isVisible >> 2 & 0x1) { //Top side Y+
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 1, 0));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 1));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					if (block.id != 2)
						colors.push_back(color);
					else
						colors.push_back(biomeColor);
				}
				if (isVisible >> 3 & 0x1) { //Bottom side Y-
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0, 0));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 0));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					colors.push_back(color);
				}
				if (isVisible >> 4 & 0x1) { //south side Z+
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 3));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					colors.push_back(color);
				}
				if (isVisible >> 5 & 0x1) { //north side Z-
					glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 1));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1, 0, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1.0f));
					models.push_back(faceTransform);
					auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 4));
					if (texture != textureAtlas.end())
						textures.push_back(texture->second);
					else
						textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
						                             0.0078125, 0.00442477876106194690)); //Fallback TNT texture
					colors.push_back(color);
				}
			}
		}
	}
	numOfFaces = textures.size();
	hash = section.GetHash();
}

void RendererSection::PrepareRender() {
	glBindBuffer(GL_ARRAY_BUFFER, VboTextures);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(glm::vec4), textures.data(), GL_DYNAMIC_DRAW);
	textures.clear();

	glBindBuffer(GL_ARRAY_BUFFER, VboModels);
	glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_DRAW);
	models.clear();

	glBindBuffer(GL_ARRAY_BUFFER, VboColors);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_DYNAMIC_DRAW);
	colors.clear();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
