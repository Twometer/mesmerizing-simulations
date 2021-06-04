//
// Created by twome on 3 Jun 2021.
//

#include <imgui.h>
#include <ctime>
#include <fstream>
#include <inipp/inipp.h>
#include "Loader.h"
#include "Renderer.h"
#include "Agent.h"
#include <portable-file-dialogs/portable-file-dialogs.h>

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
    respawn_agents();
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

    reset_values();
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

    if (showConfigMenu) {
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
        if (ImGui::Button("Respawn agents")) {
            respawn_agents();
        }
        ImGui::End();
    }

    if (showAbout) {
        bool open;
        ImGui::Begin("About", &open, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Mesmerizing Simulations 1.0.0");
        ImGui::Text("(c) 2021 made by Twometer");
        ImGui::Separator();
        ImGui::Text("Inspired by Sebastian Lague");
        ImGui::End();

        if (!open)
            showAbout = false;
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("New")) {
                reset_values();
                respawn_agents();
            }

            if (ImGui::MenuItem("Save preset")) {
                auto path = pfd::save_file("Load preset", ".", {"INI Presets", "*.ini"}).result();
            }

            if (ImGui::MenuItem("Load preset")) {
                auto path = pfd::open_file("Load preset", ".", {"INI Presets", "*.ini"}).result();
                if (!path.empty()) {
                    std::ifstream file(path[0]);
                    inipp::Ini<char> ini;
                    ini.parse(file);

                    auto agentSection = ini.sections["Agents"];
                    auto colorSection = ini.sections["Colors"];
                    inipp::get_value(agentSection, "MoveSpeed", agentSpeed);
                    inipp::get_value(agentSection, "TurnSpeed", turnSpeed);
                    inipp::get_value(agentSection, "SensorAngle", sensorAngleOffset);
                    inipp::get_value(agentSection, "SensorDist", sensorDstOffset);
                    inipp::get_value(agentSection, "SensorSize", sensorSize);
                    inipp::get_value(agentSection, "DecaySpeed", evapSpeed);
                    inipp::get_value(agentSection, "DiffusionSpeed", diffusionSpeed);
                    inipp::get_value(agentSection, "SpawnMode", spawnMode);
                }

            }

            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                exit(0);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Show config")) {
                showConfigMenu = true;
            }
            if (ImGui::MenuItem("Hide config")) {
                showConfigMenu = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("?")) {
            if (ImGui::MenuItem("About")) {
                showAbout = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Renderer::shutdown() {
    delete drawShader;
    delete agentShader;
}

float randomFloat() {
    return ((float) rand()) / ((float) RAND_MAX);
}

void Renderer::respawn_agents() {
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

void Renderer::reset_values() {
    evapSpeed = 0.45f;
    agentSpeed = 0.3f;
    diffusionSpeed = 0.5f;
    sensorAngleOffset = 0.38f;
    sensorDstOffset = 17;
    rgbLo[0] = 0;
    rgbLo[1] = 0.35;
    rgbLo[2] = 1;

    rgbHi[0] = 0;
    rgbHi[1] = 1;
    rgbHi[2] = 0.85;
    sensorSize = 15;
    turnSpeed = 0.5;
    spawnMode = 1;
    agentCount = 16384;
    showConfigMenu = true;
    showAbout = false;
}
