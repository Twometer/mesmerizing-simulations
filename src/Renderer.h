//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_RENDERER_H
#define MESMERIZING_SIMULATIONS_RENDERER_H


#include <GLFW/glfw3.h>
#include "Shader.h"

#define WIDTH 1920
#define HEIGHT 1088

class Renderer {
private:
    GLFWwindow *window;

    Shader *drawShader;

    Shader *agentShader;
    Shader *diffusionShader;
    Shader *resetShader;

    GLuint texture;

    float evapSpeed = 0.8f;
    float agentSpeed = 0.65f;
    float diffusionSpeed = 0.5f;

    float sensorAngleOffset = 0.1;
    float sensorDstOffset = 5;
    float rgb[3];
    int sensorSize = 10;
    float turnSpeed = 0.1;

    int spawnMode = 0;
public:
    explicit Renderer(GLFWwindow *window);

    void initialize();

    void render_frame();

    void shutdown();

private:
    void regen_agents();
};


#endif //MESMERIZING_SIMULATIONS_RENDERER_H
