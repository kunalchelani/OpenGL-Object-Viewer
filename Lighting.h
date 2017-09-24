#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct Light{
	Vector3f lightDirection;
	Vector3f lightColor;
	float diffuseLightIntensity;
	float ambientLightIntensity;
};

class Lighting{

public:

	GLuint gWorldLocation;
	GLuint shaderProgram;
	const char* vertexShaderFileCPU = "shader_CPU.vs";
	const char* fragmentShaderFileCPU = "shader_CPU.fs";
	const char* vertexShaderFileGPU = "shader_GPU.vs";
	const char* fragmentShaderFileGPU = "shader_GPU.fs";
	
	Lighting(char cg){
		if (cg == 'G')
			compileShadersGPU();
		else if (cg == 'C')
			compileShadersCPU();
		dirLightLocation.lightColor = glGetUniformLocation(shaderProgram,"light.lightColor");
   		dirLightLocation.ambientLightIntensity = glGetUniformLocation(shaderProgram,"light.ambientLightIntensity");
    	dirLightLocation.lightDirection = glGetUniformLocation(shaderProgram,"light.lightDirection");
    	dirLightLocation.diffuseLightIntensity = glGetUniformLocation(shaderProgram,"light.diffuseLightIntensity");
    	eyeWorldPosLocation = glGetUniformLocation(shaderProgram, "eyeWorldPos");
    	matSpecularIntensityLocation = glGetUniformLocation(shaderProgram, "matSpecularIntensity");
    	matSpecularPowerLocation = glGetUniformLocation(shaderProgram, "matSpecularPower");

	};

	void addShader(GLuint shaderProgram, const char* shaderFileText, GLenum shaderType) {
		GLuint shaderObject = glCreateShader(shaderType);

		if (shaderObject == 0) {
			fprintf(stderr, "Error creating shader type %d\n", shaderType);
			exit(0);
		}

		const GLchar * p[1];
		p[0] = shaderFileText;
		GLint lengths[1];
		lengths[0] = strlen(shaderFileText);
		glShaderSource(shaderObject, 1, p, lengths);
		glCompileShader(shaderObject);
		GLint success;
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);

		if (!success) {
			GLchar log[1024];
			glGetShaderInfoLog(shaderObject, 1024, NULL, log);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, log);
			exit(1);
		}

		glAttachShader(shaderProgram, shaderObject);
	}

	void compileShadersCPU(){

		shaderProgram = glCreateProgram();

		if (shaderProgram == 0) {
			fprintf(stderr, "Error creating shader program\n");
			exit(1);
		}

		std::string vs, fs;

		if (!ReadFile(vertexShaderFileCPU, vs)) {
			exit(1);
		}

		if (!ReadFile(fragmentShaderFileCPU, fs)) {
			exit(1);
		}

		addShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
		addShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

		GLint success = 0;
		GLchar log[1024] = {0};

		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (success == 0) {
			glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log);
			fprintf(stderr, "Error linking shader program: '%s'\n", log);
			exit(1);
		}

		/*glValidateProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log);
			fprintf(stderr, "Invalid shader program: '%s'\n", log);
			exit(1);
		}
		*/
		glUseProgram(shaderProgram);
		gWorldLocation = glGetUniformLocation(shaderProgram, "gWorld");
	}

	void compileShadersGPU(){

		shaderProgram = glCreateProgram();

		if (shaderProgram == 0) {
			fprintf(stderr, "Error creating shader program\n");
			exit(1);
		}

		std::string vs, fs;

		if (!ReadFile(vertexShaderFileGPU, vs)) {
			exit(1);
		}

		if (!ReadFile(fragmentShaderFileGPU, fs)) {
			exit(1);
		}

		addShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
		addShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

		GLint success = 0;
		GLchar log[1024] = {0};

		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (success == 0) {
			glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log);
			fprintf(stderr, "Error linking shader program: '%s'\n", log);
			exit(1);
		}

		/*glValidateProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log);
			fprintf(stderr, "Invalid shader program: '%s'\n", log);
			exit(1);
		}
		*/
		glUseProgram(shaderProgram);
		gWorldLocation = glGetUniformLocation(shaderProgram, "gWorld");
	}

	void setLightComponents(Light lightProperties){
		glUniform3f(dirLightLocation.lightColor, lightProperties.lightColor.x, lightProperties.lightColor.y, lightProperties.lightColor.z);
	    glUniform1f(dirLightLocation.ambientLightIntensity, lightProperties.ambientLightIntensity);
	    Vector3f lightDirection = lightProperties.lightDirection;
	    lightDirection.Normalize();
	    glUniform3f(dirLightLocation.lightDirection, lightDirection.x, lightDirection.y, lightDirection.z);
	    glUniform1f(dirLightLocation.diffuseLightIntensity, lightProperties.diffuseLightIntensity);
	    glUniform3f(eyeWorldPosLocation, 0.0, 0.0, 2.0);
	    glUniform1f(matSpecularIntensityLocation, 1.0);
	    glUniform1f(matSpecularPowerLocation, 10.0);
	}

private:

	GLuint eyeWorldPosLocation;
    GLuint matSpecularIntensityLocation;
    GLuint matSpecularPowerLocation;

	
	struct {
        GLuint lightColor;
        GLuint ambientLightIntensity;
        GLuint lightDirection;
        GLuint diffuseLightIntensity;
    } dirLightLocation;	
};