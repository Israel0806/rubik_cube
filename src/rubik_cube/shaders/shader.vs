#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 Color;
    vec2 texCoord;
} vs_out;

void main() {
    vs_out.Color = aColor;
    vs_out.texCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
