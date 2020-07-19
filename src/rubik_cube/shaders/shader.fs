#version 330 core
  
in vec3 Color;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord) * vec4(Color, 1.0f);
}  
