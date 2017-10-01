#version 330 core
in vec2 TexCoords;
out float fragColor;

uniform int draw_mode;
uniform sampler2D ssaoInput;

//Usamos el tamanyo de la textura de ruido
uniform int uBlurSize;

//El shader blur es bastante simple, ya que contamos con la textura con ruido
//que nos ayuda a simplificar los cálculos
void main()
{
	//Con el modo de dibujo 5, no realizamos Blur sobre la textura
	if(draw_mode == 5)
		fragColor = texture(ssaoInput, TexCoords).r;
	else
	{
		vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
		vec2 hlim = vec2(float(-uBlurSize) * 0.5 + 0.5);

		float result = 0.0;
		for (int x = 0; x < uBlurSize; ++x) 
		{
			for (int y = 0; y < uBlurSize; ++y) 
			{
				vec2 offset = (hlim + vec2(float(x), float(y))) * texelSize;
				result += texture(ssaoInput, TexCoords + offset).r;
			}
		}
		fragColor = result / (uBlurSize * uBlurSize);
	}
}  