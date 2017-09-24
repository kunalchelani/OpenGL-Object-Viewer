#define _GLIBCXX_USE_C99 1
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <limits>
#include <algorithm>

struct VertexGPU{
	Vector3f position;
	Vector3f normal;
	Vector3f texCoord;
	VertexGPU() { }
	VertexGPU(Vector3f pos) {
		position = pos;
		normal = Vector3f(0.0f, 0.0f, 0.0f);
	}
};

class OFF_GPU{

public:

	int numVertices;
	int numFaces;
	int numIndices;
	Vector3f meanProteinPosition;
	std::vector<VertexGPU> vertices; 
	std::vector<int> faceIndices;
	std::vector<Vector3f> isoContourPoints;
	std::string offFilePath;
	float extremeMax;

	OFF_GPU(std::string offFilePath){
		this->offFilePath = offFilePath;
	}

	void readOffFile(){

		float z_max = -1*std::numeric_limits<float>::max(), y_max = -1*std::numeric_limits<float>::max() , x_max = -1*std::numeric_limits<float>::max();
		float z_min =  1*std::numeric_limits<float>::max(), y_min = 1*std::numeric_limits<float>::max(), x_min = 1*std::numeric_limits<float>::max(); 

		std::string line;
		std::ifstream offFile((this->offFilePath).c_str());
		
		if (offFile.is_open()){
			getline(offFile, line);
			if (line != "OFF"){
				std::cout << "File not an OFF" << std::endl;
				return;
			}

			int numVertices, numFaces;
			getline(offFile, line);
			std::istringstream iss3(line);
			iss3 >> numVertices >> numFaces;

			this->numVertices = numVertices;
			this->numFaces = numFaces;			
			this->numIndices = 3*numFaces;	

			std::cout << "Number of vertices :" << numVertices << std::endl;
			std::cout << "Number of faces :" << numFaces << std::endl;

			Vector3f sumProteinPosition(0.0f, 0.0f, 0.0f);

			for(int i = 0; i < numVertices; i++){
				getline(offFile, line);
				float x, y, z;
				std::istringstream iss(line);
				iss >> x >> y >> z;
				this->vertices.push_back(VertexGPU(Vector3f(x, y, z)));

				if (z > z_max){z_max = z;}
				if (y > y_max){y_max = y;}
				if (x > x_max){x_max = x;}
				if (z < z_min){z_min = z;}
				if (y < y_min){y_min = y;}
				if (x < x_min){x_min = x;}
				
				sumProteinPosition += Vector3f(x, y, z);
			}

			extremeMax = max(max((z_max - z_min), (x_max -  x_min)), (y_max - y_min));
			std::cout << "Extreme Diff" << extremeMax << std::endl;

			this->meanProteinPosition = Vector3f(sumProteinPosition.x/(1.0*numVertices), sumProteinPosition.y/(1.0*numVertices), sumProteinPosition.z/(1.0*numVertices));
			std::cout << "Mean position : " << this->meanProteinPosition.x << "\t" << this->meanProteinPosition.y << "\t" << this->meanProteinPosition.z << std::endl;

			for(int i=0; i < numFaces; i++){
				getline(offFile, line);
				
				int j,a,b,c;
				std::istringstream iss2(line);
				
				iss2 >> j >> a >> b >> c;
				this->faceIndices.push_back(a);
				this->faceIndices.push_back(b);
				this->faceIndices.push_back(c);	
			}
		} 
		else std::cout << "Unable to open file" << std::endl;
	}

	void computeNormals(){
		Vector3f vec1, vec2, normal;

		for(std::vector<int>::iterator it = faceIndices.begin(); it < faceIndices.end(); it = it+3 ){
			vec1 = vertices[*(it+1)].position - vertices[*(it)].position;
			vec2 = vertices[*(it+2)].position - vertices[*(it)].position;
			normal = vec2.Cross(vec1);
			
			vertices[*it].normal += normal;
			vertices[*(it+1)].normal += normal;
			vertices[*(it+2)].normal += normal;
		}

		for(int i = 0; i < this->numVertices; i++){
			vertices[i].normal = vertices[i].normal.Normalize();
		}
	}

	void mapTexCoordinates(scalarField* field){
		for (std::vector<VertexGPU>::iterator it = vertices.begin(); it < vertices.end(); it++){
			float x = (*it).position.x;
			float y = (*it).position.y;
			float z = (*it).position.z;
			(*it).texCoord.x = max1(0, min(1, (x - field->origin[0])/(field->step[0] * field->dim[0]) ) );
			(*it).texCoord.y = max1(0, min(1, (y - field->origin[1])/(field->step[1] * field->dim[1]) ) );
			(*it).texCoord.z = max1(0, min(1, (z - field->origin[2])/(field->step[2] * field->dim[2]) ) );
			//std::cout << "Values\t" << (*it).texCoord.x << "\t" << x << std::endl; //<< "\t" << (*it).texCoord.y << "\t" << (*it).texCoord.z << std::endl;
		}
	}
};
