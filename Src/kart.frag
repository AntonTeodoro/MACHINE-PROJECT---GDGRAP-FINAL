#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
    float alpha;  
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;

void main() {
    
    vec4 texColor = texture(material.diffuse, TexCoords);
    
    float alpha = texColor.a < 0.1 ? material.alpha : texColor.a;
    
    vec3 ambient = dirLight.ambient * texColor.rgb;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * texColor.rgb;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = dirLight.specular * spec * texture(material.specular, TexCoords).rgb;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, alpha);  
    
    if (alpha <= 0.0) {
        discard;
    }
}