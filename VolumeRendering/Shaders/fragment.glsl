#version 330 core

out vec4 FragColor;
 
in vec3 TexCoord;

uniform sampler3D ourTexture;

void main()
{
	vec4 texData = texture(ourTexture, TexCoord);
	
    FragColor = texData;
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}