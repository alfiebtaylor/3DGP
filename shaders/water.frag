#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor;		

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec3 waterColor;
uniform vec3 skyColor;            

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;
	vec4 waterColor4, skyColor4;
	waterColor4 = vec4(waterColor, 1.0f);
	skyColor4 = vec4(skyColor, 1.0f);
	outColor = mix(waterColor4, skyColor4, reflFactor);
}
