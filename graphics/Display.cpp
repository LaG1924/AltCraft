#include <iostream>
#include "Display.hpp"
#include "Shader.hpp"

Display *Display::instance = nullptr;

Display::Display(int w, int h, std::string title, World *worldPtr) {
    if (instance != nullptr)
        throw 516;
    instance = this;
    world = worldPtr;
    //GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Can't create GLFW window" << std::endl;
        glfwTerminate();
        throw 517;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, &Display::callback_key);
    //GLEW
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Can't initialize GLEW: " << glewGetErrorString(glewStatus) << std::endl;
        throw 518;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}

Display::~Display() {
    instance = nullptr;
    glfwTerminate();
}

bool Display::IsClosed() {
    return false;
}

void Display::SetPlayerPos(float x, float y) {

}

void Display::MainLoop() {
    Shader vertexShader("./graphics/simpleVS.vs");
    Shader fragmentShader("./graphics/simpleFS.fs", false);
    ShaderProgram program;
    program.Attach(vertexShader);
    program.Attach(fragmentShader);
    program.Link();

    GLfloat vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f
    };
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    // 2. Копируем наш массив вершин в буфер для OpenGL
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. Устанавливаем указатели на вершинные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);
    //4. Отвязываем VAO
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram((GLuint) program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
}

void Display::callback_key(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(instance->window, GL_TRUE);
}
