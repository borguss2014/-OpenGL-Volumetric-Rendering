#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 aTexCoord;

out vec3 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraPosition;

void main(void) 
{
	TexCoord = aTexCoord;
    vec4 worldPosition = model * vec4(position, 1.0);
	gl_Position = projection * view * worldPosition;
}