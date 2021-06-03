#version 430

struct Agent {
    float x;
    float y;
    float a;
};

uniform float agentSpeed;
uniform writeonly image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;
layout (std430, binding = 3) buffer agentData
{
    Agent agents[];
};

const float PI = 3.1415;

float rand(uint x) {
    x *= 126391135u;
    x ^= 912098410u;
    x *= 126391135u;
    x ^= 912098410u;
    x *= 126391135u;
    x ^= 912098410u;
    return float(x) / 4294967295.0;
}


void main() {
    uint agentId = gl_GlobalInvocationID.x;
    Agent agent = agents[agentId];

    agent.x += sin(agent.a) * agentSpeed;
    agent.y += cos(agent.a) * agentSpeed;

    ivec2 imgSize = imageSize(destTex);

    float random = agent.y * imgSize.x + agent.x + agentId;

    if (agent.x < 0 || agent.y < 0 || agent.x >= imgSize.x || agent.y >= imgSize.y) {
        agent.x = min(imgSize.x-0.01, max(0, agent.x));
        agent.y = min(imgSize.y-0.01, max(0, agent.y));
        agent.a = random * PI * 2.0;
    }

    imageStore(destTex, ivec2(agent.x, agent.y), vec4(1, 1, 1, 1));

    agents[agentId] = agent;
}