#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>

class Texture {

public:
	Texture(GLenum TextureTarget){
		m_textureTarget = TextureTarget;
	}

	void createTexture3D(float* data, float dim1, float dim2, float dim3){
		std::cout << dim1 << "\t" << dim2 << "\t" << dim3 << std::endl;
		glGenTextures(1, &m_textureObj);
	    glBindTexture(m_textureTarget, m_textureObj);
	    glTexImage3D(m_textureTarget, 0, GL_R32F, dim1, dim2, dim3, 0, GL_RED, GL_FLOAT, data);
	    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    	glBindTexture(m_textureTarget, 0);
	}

	void createTexture1D(float* data, float dim1){
		glGenTextures(1, &m_textureObj);
	    glBindTexture(m_textureTarget, m_textureObj);
	    glTexImage1D(m_textureTarget, 0, GL_RGB, dim1, 0, GL_RGB, GL_FLOAT, data);
	    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    	glBindTexture(m_textureTarget, 0);
	}

	void Bind(GLenum TextureUnit)
	{
   		glActiveTexture(TextureUnit);
   		glBindTexture(m_textureTarget, m_textureObj);
	}

private:
	GLenum m_textureTarget;
    GLuint m_textureObj;	
};