#version 130
in vec3 Position;
in vec3 Normal;
in float scalarFieldValue;
in vec3 color;

uniform mat4 gWorld;

out vec3 normal;
out vec3 worldPos;
out float scalarFieldValueFs;
out vec3 objectColor; 

vec3 depth_color;

void main()
{   
	objectColor = color;
	scalarFieldValueFs = scalarFieldValue;
	normal = (gWorld * vec4(Normal, 0.0)).xyz;
    gl_Position = gWorld * vec4(Position, 1.0);
    worldPos = (gWorld * vec4(Position, 1.0)).xyz;
}
