#version 130
in vec3 Position;
in vec3 Normal;
in vec3 texCoord;

uniform mat4 gWorld;

out vec3 normal;
out vec3 worldPos;
out vec3 texCoordfs; 

void main()
{   
	normal = (gWorld * vec4(Normal, 0.0)).xyz;
    gl_Position = gWorld * vec4(Position, 1.0);
    worldPos = (gWorld * vec4(Position, 1.0)).xyz;
    texCoordfs = texCoord;
}
