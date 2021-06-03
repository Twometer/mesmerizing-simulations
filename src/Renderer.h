//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_RENDERER_H
#define MESMERIZING_SIMULATIONS_RENDERER_H


#include <GLFW/glfw3.h>
#include "Shader.h"

class Renderer {
private:
    GLFWwindow *window;

    Shader *drawShader;

    Shader *agentShader;
    Shader *evaporateShader;

    GLuint texture;

    float evapSpeed = 0.01f;
    float agentSpeed = 0.025f;

public:
    explicit Renderer(GLFWwindow *window);

    void initialize();

    void render_frame();

    void shutdown();
};


#endif //MESMERIZING_SIMULATIONS_RENDERER_H
