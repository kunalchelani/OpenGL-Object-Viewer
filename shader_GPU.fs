#version 130

in vec3 normal;
in vec3 worldPos;
in vec3 texCoordfs;

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
uniform sampler3D gSampler;
uniform sampler1D colorMap;
uniform float isoValue;
void main()
{

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

	vec4 scalar = texture3D(gSampler, texCoordfs);
	float epsilon = 0.008;
	//scalar = (scalar+5)/10;
	float scalarValue = scalar.x;
	if(scalarValue < -5){
		scalarValue = -5;
	}
	if(scalarValue > 5){
		scalarValue = 5;
	}
	scalarValue = (scalarValue + 5)/10;

	vec4 color1D = texture1D(colorMap, scalarValue);
	if(scalarValue < (isoValue + epsilon) && scalarValue > (isoValue - epsilon)){
			color1D  = vec4(0,0,0,1);
	}

	gl_FragColor =  color1D * (ambientColor  + diffuseColor + specularColor);
}
