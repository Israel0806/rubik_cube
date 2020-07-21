#version 330 core
out vec4 FragColor;
  
in GS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoord;
    vec3 objectColor;
} fs_in;

//uniform sampler2D texture1;

struct Material {
    sampler2D diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// lightning
uniform vec3 viewPos;
uniform Material material;
uniform Light light;


void main() {
    
    // ambient
    vec3 ambient = light.ambient * fs_in.objectColor * texture(material.diffuse, fs_in.TexCoord).rgb;
      	
    // diffuse 
    vec3 norm = fs_in.Normal;
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * fs_in.objectColor * texture(material.diffuse, fs_in.TexCoord).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0f);
}  
