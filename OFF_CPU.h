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
#include "math_utils.h"
#include "POTReader.h"
#define min(a,b) (((a)<(b))?(a):(b))
#define max1(a,b) (((a)<(b))?(b):(a))

int R[33] = {9, 68, 77, 87, 98, 108, 119, 130, 141, 152, 163, 174, 184, 194, 204,

        213, 221, 229, 236, 241, 245, 247, 247, 247, 244, 241, 236, 229, 222, 213, 203, 192, 180};

int G[33] = {76, 90, 104, 117, 130, 142, 154, 165, 176, 185, 194, 201, 208, 213, 217,

        219, 221, 216, 211, 204, 196, 187, 177, 166, 154, 141, 127, 112, 96, 80, 62, 40, 4};

int B[33] = {192, 204, 215, 225, 234, 241, 247, 251, 254, 255, 255, 253, 249, 244, 238, 230, 221,

        209, 197, 185, 173, 160, 148, 135, 123, 111, 99, 88, 77, 66, 56, 47, 38};

struct VertexCPU{
	Vector3f position;
	Vector3f normal;
	float scalarFieldValue;
	Vector3f color;
	VertexCPU() { }
	VertexCPU(Vector3f pos) {
		position = pos;
		normal = Vector3f(0.0f, 0.0f, 0.0f);
	}
};

class OFF_CPU{

public:
	int numVertices;
	int numFaces;
	int numIndices;
	Vector3f meanProteinPosition;
	std::vector<VertexCPU> vertices; 
	std::vector<int> faceIndices;
	std::vector<Vector3f> isoContourPoints;
	std::string offFilePath;
	float extremeMax;

	OFF_CPU(std::string offFilePath){
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
				this->vertices.push_back(VertexCPU(Vector3f(x, y, z)));

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

	Vector3f mapScalarToColor(float value){
		if (value < -5) value = -5;
		if (value > 5) value = 5;
		int index[2];
		float RValues[2], GValues[2], BValues[2];
		float Eps[2];
		index[0] = int(floor((16/5)*(value + 5)));
    	index[1] = min(index[0]+1, 32);

    	//std::cout << index[0]
    	for (int i = 0; i <2; i++){
    		
    		if(index[i] < 0) index[i] = 0;
    		if(index[i] > 32) index[i] = 0;

    		Eps[i] = -5.0 + index[i]*(10.0/32.0);

    		RValues[i] = (float)R[index[i]];
    		GValues[i] = (float)G[index[i]];	
    		BValues[i] = (float)B[index[i]];
    	}
    	float red = getLinearInterpolation(RValues, Eps, value)/255.0;
    	float green = getLinearInterpolation(GValues, Eps, value)/255.0;
    	float blue = getLinearInterpolation(BValues, Eps, value)/255.0;
    	//<< "value :\t" << value << "\t" <<red<< "\t" << green << "\t" << blue << "\n";
    	Vector3f returnVal(red, green, blue);
    	//Vector3f returnVal(1.0,1.0,1.0);
    	return returnVal;	
	}

	void computeScalarFieldValue(scalarField* field){
		
		for(int i = 0; i < this->numVertices; i++){
			float point[3];
			point[0] = vertices[i].position.x;
			point[1] = vertices[i].position.y;
			point[2] = vertices[i].position.z;
			float value = getValueTriLinear(field, point);
			vertices[i].scalarFieldValue = value;
			vertices[i].color = mapScalarToColor(value);
		}
	}

	void getIsoContourPoints(float contourValue){
		float reqX, reqY, reqZ;
		VertexCPU v1, v2, v3;

		isoContourPoints.clear();
		for(std::vector<int>::iterator it = faceIndices.begin(); it < faceIndices.end(); it = (it+3) ){

			v1 = vertices[*it];
			v2 = vertices[*(it+1)];
			v3 = vertices[*(it+2)];
			
			if((v1.scalarFieldValue - contourValue) * (v2.scalarFieldValue - contourValue) < 0){
				reqX = v1.position.x + (contourValue - v1.scalarFieldValue)*(v2.position.x - v1.position.x)/(v2.scalarFieldValue - v1.scalarFieldValue);
				reqY = v1.position.y + (contourValue - v1.scalarFieldValue)*(v2.position.y - v1.position.y)/(v2.scalarFieldValue - v1.scalarFieldValue);
				reqZ = v1.position.z + (contourValue - v1.scalarFieldValue)*(v2.position.z - v1.position.z)/(v2.scalarFieldValue - v1.scalarFieldValue);
				//std::cout << "isoPoint\t" << reqX << "\t" << reqY << "\t" << reqZ << "\n";
				isoContourPoints.push_back(Vector3f(reqX, reqY, reqZ));
			}
			if((v2.scalarFieldValue - contourValue) * (v3.scalarFieldValue - contourValue) < 0){
				reqX = v2.position.x + (contourValue - v2.scalarFieldValue)*(v3.position.x - v2.position.x)/(v3.scalarFieldValue - v2.scalarFieldValue);
				reqY = v2.position.y + (contourValue - v2.scalarFieldValue)*(v3.position.y - v2.position.y)/(v3.scalarFieldValue - v2.scalarFieldValue);
				reqZ = v2.position.z + (contourValue - v2.scalarFieldValue)*(v3.position.z - v2.position.z)/(v3.scalarFieldValue - v2.scalarFieldValue);
				//std::cout << "isoPoint\t" << reqX << "\t" << reqY << "\t" << reqZ << "\n";
				isoContourPoints.push_back(Vector3f(reqX, reqY, reqZ));
			}
			if((v3.scalarFieldValue - contourValue) * (v1.scalarFieldValue - contourValue) < 0){
				reqX = v3.position.x + (contourValue - v3.scalarFieldValue)*(v1.position.x - v3.position.x)/(v1.scalarFieldValue - v3.scalarFieldValue);
				reqY = v3.position.y + (contourValue - v3.scalarFieldValue)*(v1.position.y - v3.position.y)/(v1.scalarFieldValue - v3.scalarFieldValue);
				reqZ = v3.position.z + (contourValue - v3.scalarFieldValue)*(v1.position.z - v3.position.z)/(v1.scalarFieldValue - v3.scalarFieldValue);
				//std::cout << "isoPoint\t" << reqX << "\t" << reqY << "\t" << reqZ << "\n";
				isoContourPoints.push_back(Vector3f(reqX, reqY, reqZ));
			}
		}
		std::cout << " Number of iso points: " << isoContourPoints.size() << std::endl;
	}

};
