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
layout (local_size_x = 1024) in;
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

float sense(Agent agent, float angleOffset) {
    float sensorAngle = agent.a + angleOffset;
    vec2 sensorDir = vec2(cos(sensorAngle), sin(sensorAngle)) * sensorDstOffset;
    ivec2 sensorCtr = ivec2(agent.x + sensorDir.x, agent.y + sensorDir.y);

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

    agent.x += cos(agent.a) * agentSpeed;
    agent.y += sin(agent.a) * agentSpeed;

    ivec2 imgSize = imageSize(destTex);
    uint hashedId = uint(rand(uint(agentId * 12345)));
    float random = rand(uint(agent.y * imgSize.x + agent.x + hashedId));

    if (agent.x < 0 || agent.y < 0 || agent.x >= imgSize.x || agent.y >= imgSize.y) {
        agent.x = min(imgSize.x - 0.01, max(0, agent.x));
        agent.y = min(imgSize.y - 0.01, max(0, agent.y));
        agent.a = random * PI * 2.0;
    }

    float wFwd = sense(agent, 0);
    float wLeft = sense(agent, sensorAngleOffset);
    float wRight = sense(agent, -sensorAngleOffset);
    if (wFwd > wLeft && wFwd > wRight) {
        // do nothing
    } else if (wFwd < wLeft && wFwd < wRight) {
        agent.a += (random - 0.5) * 2 * turnSpeed;
    } else if (wRight > wLeft) {
        agent.a -= random * turnSpeed;
    } else if (wLeft > wRight) {
        agent.a += random * turnSpeed;
    }

    imageStore(destTex, ivec2(agent.x, agent.y), vec4(1, 1, 1, 1));
    agents[agentId] = agent;
}