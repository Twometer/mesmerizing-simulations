//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_RENDERER_H
#define MESMERIZING_SIMULATIONS_RENDERER_H


#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Agent.h"

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

    float evapSpeed = 0.45f;
    float agentSpeed = 0.3f;
    float diffusionSpeed = 0.5f;

    float sensorAngleOffset = 0.38;
    float sensorDstOffset = 17;
    float rgbLo[3];
    float rgbHi[3];
    int sensorSize = 15;
    float turnSpeed = 0.5;

    int spawnMode = 1;
    int agentCount = 16384;

    bool showConfigMenu = true;
    bool showAbout = false;

    Agent *agents = nullptr;
    int actualNumAgents = 0;
public:
    explicit Renderer(GLFWwindow *window);

    void initialize();

    void render_frame();

    void shutdown();

private:
    void respawn_agents();

    void reset_values();
};


#endif //MESMERIZING_SIMULATIONS_RENDERER_H
