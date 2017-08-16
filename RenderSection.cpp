#include <graphics/RenderSection.hpp>

const GLfloat vertices[] = {
		//Z+ edge
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,

		//Z- edge
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,

		//X+ edge
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		//X- edge
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,

		//Y+ edge
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,

		//Y- edge
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
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

void RenderState::SetActiveVao(GLuint Vao) {
	if (Vao != ActiveVao) {
		glBindVertexArray(Vao);
		ActiveVao = Vao;
	}
}

void RenderState::SetActiveShader(GLuint Shader) {
	if (Shader != ActiveShader) {
		glUseProgram(Shader);
		ActiveShader = Shader;
	}
}

const GLuint magicUniqueConstant = 88375;
GLuint RenderSection::VboVertices = magicUniqueConstant;
GLuint RenderSection::VboUvs = magicUniqueConstant;
std::map<GLuint, int> RenderSection::refCounterVbo;
std::map<GLuint, int> RenderSection::refCounterVao;


RenderSection::RenderSection(World *world, Vector position) : sectionPosition(position), world(world) {

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

	glGenBuffers(1, &VboBlocks);
	if (refCounterVbo.find(VboBlocks) == refCounterVbo.end())
		refCounterVbo[VboBlocks] = 0;
	refCounterVbo[VboBlocks]++;

	glGenBuffers(1, &VboModels);
	if (refCounterVbo.find(VboModels) == refCounterVbo.end())
		refCounterVbo[VboModels] = 0;
	refCounterVbo[VboModels]++;

	glGenVertexArrays(1, &Vao);
	if (refCounterVao.find(Vao) == refCounterVao.end())
		refCounterVao[Vao] = 0;
	refCounterVao[Vao]++;

	glBindVertexArray(Vao);
	{
		//Cube vertices
		glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(0);

		//Cube UVs
		glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(2);

		//Blocks ids
		glBindBuffer(GL_ARRAY_BUFFER, VboBlocks);
		glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(7);
		glVertexAttribDivisor(7, 1);
		glCheckError();

		//Blocks models
		size_t sizeOfMat4 = 4 * 4 * sizeof(GLfloat);
		glBindBuffer(GL_ARRAY_BUFFER, VboModels);
		glVertexAttribPointer(8 + 0, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, nullptr);
		glVertexAttribPointer(8 + 1, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (1 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(8 + 2, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (2 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(8 + 3, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *) (3 * 4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(8 + 0);
		glEnableVertexAttribArray(8 + 1);
		glEnableVertexAttribArray(8 + 2);
		glEnableVertexAttribArray(8 + 3);
		glVertexAttribDivisor(8 + 0, 1);
		glVertexAttribDivisor(8 + 1, 1);
		glVertexAttribDivisor(8 + 2, 1);
		glVertexAttribDivisor(8 + 3, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
	UpdateState();
	glCheckError();
}

RenderSection::~RenderSection() {
	refCounterVbo[VboBlocks]--;
	refCounterVbo[VboModels]--;
	refCounterVao[Vao]--;
	if (refCounterVbo[VboBlocks] <= 0)
		glDeleteBuffers(1, &VboBlocks);
	if (refCounterVbo[VboModels] <= 0)
		glDeleteBuffers(1, &VboBlocks);
	if (refCounterVao[Vao] <= 0)
		glDeleteVertexArrays(1, &Vao);
}

void RenderSection::UpdateState() {
	Section *section = &world->sections.find(sectionPosition)->second;
	std::vector<glm::mat4> models;
	std::vector<glm::vec2> blocks;
	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				Block block = section->GetBlock(Vector(x, y, z));
				if (block.id == 0)
					continue;

				unsigned char isVisible = 0;
				if (x == 0 || x == 15 || y == 0 || y == 15 || z == 0 || z == 15) {
					isVisible = 0;
				} else {
					isVisible |= (section->GetBlock(Vector(x + 1, y, z)).id != 0) << 0;
					isVisible |= (section->GetBlock(Vector(x - 1, y, z)).id != 0) << 1;
					isVisible |= (section->GetBlock(Vector(x, y + 1, z)).id != 0) << 2;
					isVisible |= (section->GetBlock(Vector(x, y - 1, z)).id != 0) << 3;
					isVisible |= (section->GetBlock(Vector(x, y, z + 1)).id != 0) << 4;
					isVisible |= (section->GetBlock(Vector(x, y, z - 1)).id != 0) << 5;
				}
				if (isVisible == 0x3F)
					continue;

				glm::vec2 data(block.id, block.state);
				blocks.push_back(data);
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(section->GetPosition().GetX() * 16,
				                                        section->GetPosition().GetY() * 16,
				                                        section->GetPosition().GetZ() * 16));
				model = glm::translate(model, glm::vec3(x, y, z));
				double size = 0.999;
				model = glm::scale(model, glm::vec3(size, size, size));
				models.push_back(model);
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, VboBlocks);
	glBufferData(GL_ARRAY_BUFFER, blocks.size() * sizeof(glm::vec2), blocks.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VboModels);
	glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	numOfBlocks = blocks.size();
}

void RenderSection::Render(RenderState &state) {
	state.SetActiveVao(Vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, numOfBlocks);
	glCheckError();
}

Section *RenderSection::GetSection() {
	return &world->sections.find(sectionPosition)->second;
}

RenderSection::RenderSection(const RenderSection &other) {
	this->world = other.world;
	this->VboModels = other.VboModels;
	this->VboBlocks = other.VboBlocks;
	this->sectionPosition = other.sectionPosition;
	this->Vao = other.Vao;
	this->numOfBlocks = other.numOfBlocks;

	refCounterVbo[VboBlocks]++;
	refCounterVbo[VboModels]++;
	refCounterVao[Vao]++;
}
