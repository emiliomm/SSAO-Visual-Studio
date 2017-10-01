#version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

//128 es el tamanyo maximo del vector samples
//Si se quiere cambiar el numero maximo posible, cambiar aqui y en el cpp del programa
uniform vec3 samples[128];

//Parametros personalizables
//valor inicial = 64
uniform int kernelSize = 64;
//valor inicial = 0.5
uniform float radius;
//valor inicial = 0.025
uniform float bias;
//valor inicial = 1.0;
uniform float power;

//Textura de ruido = Dimensiones de pantalla / tamanyo de textura de ruido
uniform vec2 noiseScale;

uniform mat4 projection;

void main()
{
    //Captamos informacion de la escena del GBuffer + la textura de ruido
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	//Creamos una matriz TBN que cambia del espacio tangente al espacio de la vista
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    //Calculamos el factor de oclusion recurriendo en bucle el SSAO kernel
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 sample = TBN * samples[i];
        sample = fragPos + sample * radius; 
        
		//Proyectamos la posición del sample hacia la textura para recoger la posición en pantalla
        vec4 offset = vec4(sample, 1.0);

		//Del espacio de vista al espacio de clip
        offset = projection * offset;

		//División de perspectiva
        offset.xyz /= offset.w;

		//Por ultimo, transformamos el rango en [0, 1]
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(gPosition, offset.xy).z; //Cogemos el valor de profundidad del valor del kernel
        
        //Comprobamos si estamos dentro del rango y si lo estamos acumulamos oclusion
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;  
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = pow(occlusion, power);
}