#version 330 core

out vec4 FragColor;
 
in vec3 TexCoord;

uniform sampler3D ourTexture;

void main()
{
	vec4 texData = texture3D(ourTexture, TexCoord);
	
    FragColor = texData;
}