#version 130

in vec3 normal;
in vec3 worldPos;
in float scalarFieldValueFs;
in vec3 objectColor;

float z_val;

struct Light{
	vec3 lightDirection;
	vec3 lightColor;
	float diffuseLightIntensity;
	float ambientLightIntensity;
};

uniform Light light;
uniform float matSpecularIntensity;
uniform float matSpecularPower;
uniform vec3 eyeWorldPos;

void main()
{
	/*
	int R[33] = int[33](59, 68, 77, 87, 98, 108, 119, 130, 141, 152, 163, 174, 184, 194, 204,

        213, 221, 229, 236, 241, 245, 247, 247, 247, 244, 241, 236, 229, 222, 213, 203, 192, 180);

	int G[33] = int[33](76, 90, 104, 117, 130, 142, 154, 165, 176, 185, 194, 201, 208, 213, 217,

        219, 221, 216, 211, 204, 196, 187, 177, 166, 154, 141, 127, 112, 96, 80, 62, 40, 4);

	int B[33] = int[33](192, 204, 215, 225, 234, 241, 247, 251, 254, 255, 255, 253, 249, 244, 238, 230, 221,

        209, 197, 185, 173, 160, 148, 135, 123, 111, 99, 88, 77, 66, 56, 47, 38);

    int index = int(floor((16/5)*(scalarFieldValueFs + 5)));
    if ( index < 0 ) index = 0;
    if ( index > 32) index = 32;

    red linear

    vec4 objectColor = vec4(float(R[index]/255.0), float(G[index]/255.0), float(B[index]/255.0), 1);
    */	

	vec4 ambientColor = vec4(light.lightColor, 1.0) * light.ambientLightIntensity;
	
	float diffusefactor = dot(normalize(normal) , -light.lightDirection);

	vec4 diffuseColor = vec4(0,0,0,0);
	vec4 specularColor = vec4(0,0,0,0);

	if(diffusefactor != 0){
		diffuseColor = vec4(diffusefactor * light.diffuseLightIntensity * light.lightColor, 1.0);
		vec3 vertextToEye = normalize(eyeWorldPos - worldPos);
		vec3 lightReflect = normalize(reflect(light.lightDirection, normalize(normal)));
		float specularFactor = dot(vertextToEye, lightReflect);
		if(specularFactor != 0){
			specularFactor = pow(specularFactor, matSpecularPower);
			specularColor = vec4(specularFactor * matSpecularIntensity * light.lightColor, 1.0);
		}
	} 

    gl_FragColor =   vec4(objectColor,1.0) *(ambientColor  + diffuseColor + specularColor ) ; //objectColor *
}
