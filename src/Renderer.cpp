//
// Created by twome on 3 Jun 2021.
//

#include <imgui.h>
#include "Loader.h"
#include "Renderer.h"

Renderer::Renderer(GLFWwindow *window) : window(window) {}

void Renderer::initialize() {
    // Shaders
    drawShader = Loader::load_draw_shader("res/draw.vert", "res/draw.frag");
    computeShader = Loader::load_compute_shader("res/compute.glsl");

    // Textures
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

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
}

void Renderer::render_frame() {
    computeShader->bind();
    computeShader->dispatch(512 / 16, 512 / 16, 1);

    drawShader->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    ImGui::Text("Hello!");
}

void Renderer::shutdown() {
    delete drawShader;
    delete computeShader;
}
