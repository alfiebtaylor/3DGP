#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec3 fogColour;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// View Matrix
uniform mat4 matrixView;

// spot light

struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
};
uniform SPOT lightSpot, lightSpot1;

vec4 SpotLight(SPOT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// spot angles and the Spot Factor
	vec3 D = mat3(matrixView) * light.direction;
	float spotFactor = dot(-L, D);
	float angle = acos(spotFactor);
	float cutoff = radians(clamp(light.cutoff, 0, 90));
	if (angle <= cutoff)
		spotFactor = pow(spotFactor, light.attenuation);
	else
		spotFactor = 0;

	return spotFactor * color;
}

void main(void) 
{
	outColor = color;
	if (lightSpot.on == 1) 
		outColor += SpotLight(lightSpot);
	if (lightSpot1.on == 1) 
		outColor += SpotLight(lightSpot1);

	outColor *= texture(texture0, texCoord0.st);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);

}
