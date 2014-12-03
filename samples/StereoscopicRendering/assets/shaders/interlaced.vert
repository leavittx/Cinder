#version 130

uniform mat4	ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec4 ciColor;

out vec4 vColor;
out vec2 vTexCoord0;

void main()
{
	vColor = ciColor;
	vTexCoord0 = ciTexCoord0;
	
	// vertex shader must always pass projection space position
	gl_Position = ciModelViewProjection * ciPosition;
}
