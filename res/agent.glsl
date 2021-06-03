#version 430

struct Agent {
    float x;
    float y;
    float a;
};

uniform float agentSpeed;
uniform float sensorAngleOffset;
uniform float sensorDstOffset;
uniform int sensorSize;
uniform float turnSpeed;

layout (binding = 0, rgba32f) uniform image2D destTex;
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

float sense(Agent agent, float sensorAngleOffset) {
    float sensorAngle = agent.a + sensorAngleOffset;
    vec2 sensorDir = vec2(sin(agent.a), cos(agent.a));
    ivec2 sensorCtr = ivec2(agent.x, agent.y) + ivec2(sensorDir * sensorDstOffset);

    float sum = 0;
    for (int ox = -sensorSize; ox <= sensorSize; ox++) {
        for (int oy = -sensorSize; oy <= sensorSize; oy++) {
            ivec2 pos = sensorCtr + ivec2(ox, oy);
            sum += imageLoad(destTex, pos).x;
        }
    }
    return sum;
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

    float wFwd = sense(agent, 0);
    float wLeft = sense(agent, sensorAngleOffset);
    float wRight = sense(agent, -sensorAngleOffset);
    if (wFwd < wLeft && wFwd < wRight) {
        agent.a += (random - 0.5) * 2 * turnSpeed;
    } else if (wRight > wLeft) {
        agent.a -= random * turnSpeed;
    } else if (wLeft > wRight) {
        agent.a += random * turnSpeed;
    }

    imageStore(destTex, ivec2(agent.x, agent.y), vec4(1, 1, 1, 1));
    agents[agentId] = agent;
}