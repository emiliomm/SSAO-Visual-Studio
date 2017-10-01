#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 uModelViewMatrix;
uniform mat4 uModelViewProjMatrix;
uniform mat3 uNormalMatrix;

//Un shader simple de geometrñia donde pasamos
//las coordenadas de posición, textura y normales
//para ponerlo en el GBuffer en el shader de geometría
//de fragmentos
void main()
{
	vec4 viewPos = uModelViewMatrix * vec4(position, 1.0f);

    FragPos = viewPos.xyz; 
	gl_Position = uModelViewProjMatrix * vec4(position, 1.0f);;
    TexCoords = texCoords;

	Normal = normalize(uNormalMatrix * normal);
}