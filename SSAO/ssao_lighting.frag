#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
uniform Light light;
uniform int draw_mode;

void main()
{             
    //Recogemos la información del GBuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    
    //Calculo de la iluminación
	//Ambiente
	vec3 ambient;
	if(draw_mode == 0)
		ambient = vec3(0.3 * Diffuse);
	else
		ambient = vec3(0.3 * Diffuse * AmbientOcclusion);

    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // Viewpos is (0.0.0)

    // Difusa
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;

    // Especular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;

    //Atenuación según distancia
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    //Dependiendo del modo de dibujado, mostramos los diferentes buffers
    if(draw_mode == 0 || draw_mode == 1)
        FragColor = vec4(lighting, 1.0);
    else if(draw_mode == 2)
        FragColor = vec4(FragPos, 1.0);
    else if(draw_mode == 3)
        FragColor = vec4(Normal, 1.0);
	else if(draw_mode == 4)
        FragColor = vec4(Diffuse, 1.0);
    else if(draw_mode == 5 || draw_mode == 6)
        FragColor = vec4(vec3(AmbientOcclusion), 1.0);
}