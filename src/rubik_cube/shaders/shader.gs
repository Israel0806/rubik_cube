
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 Color;
    vec2 texCoord;
} gs_in[];

out GS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoord;
    vec3 objectColor;
} gs_out;

vec3 GetNormal() {
    /// vector BA
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   /// vector BC
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

void main() {
    vec3 normal = GetNormal();
    for(int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;

        gs_out.Normal = normal;
        gs_out.objectColor = gs_in[i].Color;
        gs_out.TexCoord = gs_in[i].texCoord;
        gs_out.FragPos = vec3(gl_in[i].gl_Position);
        EmitVertex();
    }
    EndPrimitive();
}

    
