//
// Created by twome on 3 Jun 2021.
//

#include <imgui.h>
#include <ctime>
#include "Loader.h"
#include "Renderer.h"
#include "Agent.h"

#define PI (3.14159265359f)
#define PI_2 (2.0f * PI)
#define NUM_AGENTS 16384

Renderer::Renderer(GLFWwindow *window) : window(window) {}

void Renderer::initialize() {
    // Seed crand
    srand(time(nullptr));

    // Shaders
    drawShader = Loader::load_draw_shader("res/draw.vert", "res/draw.frag");
    agentShader = Loader::load_compute_shader("res/agent.glsl");
    diffusionShader = Loader::load_compute_shader("res/diffuse.glsl");
    resetShader = Loader::load_compute_shader("res/reset.glsl");

    // Textures
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Agent SSBO
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    regen_agents();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    // Fullscreen quad
    GLuint vertArray;
    glGenVertexArrays(1, &vertArray);
    glBindVertexArray(vertArray);

    GLuint posBuf;
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    float data[] = {
            -1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Color init
    rgbLo[0] = 1;
    rgbLo[1] = 1;
    rgbLo[2] = 1;

    rgbHi[0] = 1;
    rgbHi[1] = 1;
    rgbHi[2] = 1;
}

void Renderer::render_frame() {
    agentShader->bind();
    agentShader->set("agentSpeed", agentSpeed);
    agentShader->set("sensorAngleOffset", sensorAngleOffset);
    agentShader->set("sensorDstOffset", sensorDstOffset);
    agentShader->set("sensorSize", sensorSize);
    agentShader->set("turnSpeed", turnSpeed);
    agentShader->dispatch(NUM_AGENTS / 1024, 1, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    diffusionShader->bind();
    diffusionShader->set("diffusionSpeed", diffusionSpeed);
    diffusionShader->set("evapSpeed", evapSpeed);
    diffusionShader->dispatch(WIDTH / 16, HEIGHT / 16, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    drawShader->bind();
    drawShader->set("darkColor", glm::vec3(rgbLo[0], rgbLo[1], rgbLo[2]));
    drawShader->set("brightColor", glm::vec3(rgbHi[0], rgbHi[1], rgbHi[2]));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    ImGui::Begin("Agents");
    ImGui::SliderFloat("Move speed", &agentSpeed, 0.1, 1.0);
    ImGui::SliderFloat("Turn speed", &turnSpeed, 0.0, 1.0);
    ImGui::SliderFloat("Sensor angle", &sensorAngleOffset, -0.5, 0.5);
    ImGui::SliderFloat("Sensor dist", &sensorDstOffset, 0.0, 50);
    ImGui::SliderInt("Sensor size", &sensorSize, 0, 50);
    ImGui::End();

    ImGui::Begin("Trails");
    ImGui::SliderFloat("Decay speed", &evapSpeed, 0.00, 1.0);
    ImGui::SliderFloat("Diffusion speed", &diffusionSpeed, 0.0, 1.0);
    ImGui::End();

    ImGui::Begin("Grid");
    ImGui::SliderFloat3("Low RGB", rgbLo, 0, 1);
    ImGui::SliderFloat3("High RGB", rgbHi, 0, 1);
    ImGui::Combo("Spawn mode", &spawnMode, "Center random\0Center circle\0Random");
    if (ImGui::Button("Reset and Respawn")) {
        regen_agents();
    }

    ImGui::End();
}

void Renderer::shutdown() {
    delete drawShader;
    delete agentShader;
}

float randomFloat() {
    return ((float) rand()) / ((float) RAND_MAX);
}

void Renderer::regen_agents() {
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    resetShader->bind();
    resetShader->dispatch(WIDTH / 16, HEIGHT / 16, 1);

    Agent agents[NUM_AGENTS];
    for (int i = 0; i < NUM_AGENTS; i++) {
        if (spawnMode == 0) {
            agents[i] = Agent{display_w / 2.f, display_h / 2.f, randomFloat() * PI_2};
        } else if (spawnMode == 1) {
            float diskRadius = 250.0f;
            glm::vec2 pos = glm::vec2(randomFloat() * 2 - 1, randomFloat() * 2 - 1);
            pos = glm::normalize(pos) * diskRadius * randomFloat();
            float x = pos.x;
            float y = pos.y;

            float a = atan2(y, x) + PI;
            x += (float) display_w / 2;
            y += (float) display_h / 2;

            agents[i] = Agent{x, y, a};
        } else if (spawnMode == 2) {
            agents[i] = Agent{randomFloat() * display_w, randomFloat() * display_h,
                              randomFloat() * PI_2};
        }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(agents), agents, GL_STATIC_READ);
}
