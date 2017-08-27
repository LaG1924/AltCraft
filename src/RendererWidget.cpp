#include "RendererWidget.hpp"

const GLfloat vertices[] = {
    0.0f,0.0f,0.0f,    
    1.0f,1.0f,0.0f,
    0.0f,1.0f,0.0f,

    0.0f,0.0f,0.0f,
    1.0f,0.0f,0.0f,
    1.0f,1.0f,0.0f,
};

const GLfloat uvs[] = {
    0.0f,0.0f,
    1.0f,1.0f,
    0.0f,1.0f,    

    0.0f,0.0f,    
    1.0f,0.0f,
    1.0f,1.0f,
};

static GLuint VboVertices, VboUvs, Vao;
static Shader* guiShader = nullptr;

RendererWidget::RendererWidget(RootWidget *widget) {
    this->tree = widget;

    if (guiShader == nullptr) {
        guiShader = new Shader("./shaders/gui.vs", "./shaders/gui.fs");
        guiShader->Use();
        glUniform1i(glGetUniformLocation(guiShader->Program, "textureAtlas"), 0);        
        
        glGenBuffers(1, &VboVertices);
        glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &VboUvs);
        glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

        glGenVertexArrays(1, &Vao);
        glBindVertexArray(Vao);
        {
            glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);
        }
        glBindVertexArray(0);
    }
}

RendererWidget::~RendererWidget() {

}

void RendererWidget::Render(RenderState &state) {
    state.SetActiveVao(Vao);
    state.SetActiveShader(guiShader->Program);

    auto toRender = tree->GetRenderList();
    for (auto& it : toRender) {
        auto[x, y, w, h] = it->GetTexture();
        glUniform4f(glGetUniformLocation(guiShader->Program, "widgetTexture"), x, y, w, h);

        glUniform4f(glGetUniformLocation(guiShader->Program, "transform"), it->x, it->y, it->w, it->h);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glCheckError();
}