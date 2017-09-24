//Author Kunal Chelani kunalchelani@iisc.ac.in

//	#define _GLIBCXX_USE_C99 1
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <time.h>
#include "file_utils.h"
#include "OFF_CPU.h"
#include "OFF_GPU.h"
#include "Lighting.h"
#include "Texture.h"


//Global Variables
GLuint VAO;
GLuint VBO, ICVBO;
GLuint IBO;
GLuint gSamplerLoc, gSampler1Loc;
GLuint isoValueLoc;
OFF_CPU* offFile;
float viewAngle = 45.0;
float isoValue = 0.2;
char mode;

Texture* texture = NULL;
Texture* texture1D = NULL;
Lighting* lighting;
Light light;

std::string offFilePath = "protein_scalar_data/1grm.off";
std::string potFilePath = "protein_scalar_data/1grm.pot";
std::string title = "Protein Mesh";
ScalarField *field;

std::vector<int> contourIndices;
int windowWidth = 1000;
int windowHeight = 800;
int numVertices;
int numFaces;
int numIndices;
int isoCountourSize;

float rotation = 0.8f;
float globeRadius = 1.0f;
float prps;
float extremeMax;
float colorMapData[99];
float xDisplace = 0, yDisplace = 0;
Vector3f mouseInitial;
Vector3f meanProteinPosition;
Matrix4f rotationMatrix = Matrix4f(1, 0, 0, 0,
									0, 1, 0, 0,
									0, 0, 1, 0,
									0, 0, 0, 1);

Matrix4f previousMatrix = Matrix4f(1, 0, 0, 0,
									0, 1, 0, 0,
									0, 0, 1, 0,
									0, 0, 0, 1);

bool trackingMouse;

void updateRotation(int p, int q){
	//normalizeXY(int x, int y);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float a = 2.0*((float)p - (float)windowWidth/2.0)/(float)windowWidth;
	float b = -2.0*((float)q - (float)windowHeight/2.0)/(float)windowHeight;

	//std :: cout << "globeRadius " << globeRadius << std::endl;
	float z_new = sqrt(1.0 - a*a - b*b);
	Vector3f mouseNew = Vector3f(a, b, z_new);
	//std::cout << "z_new " << z_new << std::endl; 
	
	Vector3f normal = mouseNew.Cross(mouseInitial);
	//std::cout << "normal length" << normal.length() << std::endl;
	float angle = normal.length()/(mouseInitial.length()*mouseNew.length());
	//std::cout << "The angle is :" << angle << std::endl;
	normal = normal.Normalize();

	rotationMatrix.InitAxisRotateTransform(normal.Normalize(), angle);
	rotationMatrix = rotationMatrix * previousMatrix;
}

static void onMouseMotion(int x, int y){
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float a = 2.0*(float(x) - (float)windowWidth/2.0)/(float)windowWidth;
	float b = -2.0*(float(y) - (float)windowHeight/2.0)/(float)windowHeight;
	bool continueTracking = false;
	if(fabs(a)*fabs(a) + fabs(b)*fabs(b) < globeRadius)
		continueTracking = true;
	if(trackingMouse && continueTracking){	
		updateRotation(x, y);							
		glutPostRedisplay();
	}
}

static void onMousePassiveMotion(int x, int y){
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float a = 2.0*(float(x) - (float)windowWidth/2.0)/(float)windowWidth;
	float b = -2.0*(float(y) - (float)windowHeight/2.0)/(float)windowHeight;
	float z_new = sqrt(globeRadius - a*a - b*b);
}


static void onMouseButtonPress(int button, int state, int x, int y){

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {	
		int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
		int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
		float a = 2.0*(float(x) - (float)windowWidth/2.0)/(float)windowWidth;
		float b = -2.0*(float(y) - (float)windowHeight/2.0)/(float)windowHeight;
		//globeRadius = ( windowHeight > windowWidth ) ? windowWidth : windowHeight;
		globeRadius = 1.0;
		if(fabs(a)*fabs(a) + fabs(b)*fabs(b) < globeRadius){
			trackingMouse = true;
			std::cout << "X and Y :" << a << "\t" << b << std::endl;
			mouseInitial = Vector3f(a, b, sqrt(globeRadius - a*a - b*b));
		}	
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		trackingMouse = false;
 		previousMatrix = rotationMatrix;
	}
}

void computeFPS() {
	static int frameCount = 0;
	static int lastFrameTime = 0;
	float frameRate;
	int currentTime;
	title = "Protein Mesh ";
	frameCount++;
	currentTime = glutGet((GLenum) (GLUT_ELAPSED_TIME));
	if (currentTime - lastFrameTime > 1000) {
		frameRate  = frameCount * 1000.0 / (currentTime - lastFrameTime);
		std::ostringstream ss1,ss2;
		ss1 << frameRate;
		ss2 << prps;
		
		title.append("  FPS : [");
		title.append(ss1.str());
		title.append("]");
		title.append("  PRPS : [");
		title.append(ss2.str());
		title.append("]");

		glutSetWindowTitle(title.c_str());
		lastFrameTime = currentTime;
		frameCount = 0;
	}
}

static void renderCallbackGPU(){

	computeFPS();
	static float scaling_factor;
	PersProjInfo persProj(viewAngle, 1,1, 1.0f, 100.0f);
	scaling_factor = 1.0/extremeMax; 														//Scaling based on the avg of diff between extreme values along each axis
	static float a = 0;
	a +=0.01;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	static Matrix4f  World, scale, rotate, translate, perspectiveProj, translateMean;

	perspectiveProj.InitPersProjTransform(persProj);
	translateMean.InitTranslationTransform(-1*meanProteinPosition.x, -1*meanProteinPosition.y, -1*meanProteinPosition.z);
	translate.InitTranslationTransform(xDisplace,yDisplace,2.0);
	scale.InitScaleTransform(scaling_factor,scaling_factor,scaling_factor);
	World.InitIdentity();
	World = perspectiveProj*translate*rotationMatrix*scale*translateMean;
	
	glUniformMatrix4fv(lighting->gWorldLocation, 1, GL_TRUE, &World.m[0][0]);
	lighting->setLightComponents(light);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (const GLvoid*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (const GLvoid*)24);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    
    texture->Bind(GL_TEXTURE0);
    texture1D->Bind(GL_TEXTURE5);
    
    clock_t t;
    glFinish();
    t = clock();
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glFinish();
    t = clock() - t;
    prps = numFaces/((float)t/CLOCKS_PER_SEC);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR) {
		/* double-buffering - swap the back and front buffers */
		glutSwapBuffers();
	} else {
		
		fprintf(stderr, "OpenGL rendering error %d\n", errorCode);
	}
}

static void renderCallbackCPU(){

	computeFPS();
	static float scaling_factor;
	PersProjInfo persProj(viewAngle, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 1.0f, 100.0f);
	scaling_factor = 1.0/extremeMax; 														//Scaling based on the avg of diff between extreme values along each axis
	static float a = 0;
	a +=0.01;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	static Matrix4f  World, scale, rotate, translate, perspectiveProj, translateMean;

	perspectiveProj.InitPersProjTransform(persProj);
	translateMean.InitTranslationTransform(-1*meanProteinPosition.x, -1*meanProteinPosition.y, -1*meanProteinPosition.z);
	translate.InitTranslationTransform(xDisplace,yDisplace,2.0);
	scale.InitScaleTransform(scaling_factor,scaling_factor,scaling_factor);
	World.InitIdentity();
	World = perspectiveProj*translate*rotationMatrix*scale*translateMean;
	
	glUniformMatrix4fv(lighting->gWorldLocation, 1, GL_TRUE, &World.m[0][0]);
	lighting->setLightComponents(light);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCPU), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCPU), (const GLvoid*)12);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(VertexCPU), (const GLvoid*)24);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCPU), (const GLvoid*)28);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    
    clock_t t;
    glFinish();
    t = clock();
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glFinish();
    t = clock() - t;
    prps = numFaces/((float)t/CLOCKS_PER_SEC);
    glDisableVertexAttribArray(0);	
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, ICVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), 0);
    glLineWidth(3.0);
    glDrawArrays(GL_LINES, 0, isoCountourSize);
    glDisableVertexAttribArray(0);

    GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR) {
		/* double-buffering - swap the back and front buffers */
		glutSwapBuffers();
	} else {
		
		fprintf(stderr, "OpenGL rendering error %d\n", errorCode);
	}
}

static void createProteinVertexBufferCPU(OFF_CPU* offFile){
	std::cout << "Creating Protein Buffer" <<std::endl;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexCPU)*offFile->vertices.size(), &(offFile->vertices.front()) , GL_STATIC_DRAW);
}

static void createProteinVertexBufferGPU(OFF_GPU* offFile){
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexGPU)*offFile->vertices.size(), &(offFile->vertices.front()) , GL_STATIC_DRAW);
}

static void createIsoVertexBuffer(OFF_CPU* offFile){
	glGenBuffers(1, &ICVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ICVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f)*(offFile->isoContourPoints.size()), &(offFile->isoContourPoints.front()) , GL_STATIC_DRAW);
}

static void createIndexBufferCPU(OFF_CPU* offFile){
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(int), &(offFile->faceIndices.front()), GL_STATIC_DRAW);
}

static void createIndexBufferGPU(OFF_GPU* offFile){
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(int), &(offFile->faceIndices.front()), GL_STATIC_DRAW);
}

void prepareColorMap(){
	for (int i = 0; i < 33; i++){
		colorMapData[3*i] = ((float)R[i])/255.0;
		colorMapData[3*i + 1] = ((float)G[i])/255.0;
		colorMapData[3*i + 2] = ((float)B[i])/255.0;
	}
}

static void onIdle() {
	glutPostRedisplay();
}

static void initTextures(){

	texture = new Texture(GL_TEXTURE_3D);
	texture->createTexture3D(field->data, field->dim[0], field->dim[1], field->dim[2]);

	gSamplerLoc = glGetUniformLocation(lighting->shaderProgram, "gSampler");
	glUniform1i(gSamplerLoc, 0);
	gSampler1Loc = glGetUniformLocation(lighting->shaderProgram, "colorMap");
 	glUniform1i(gSampler1Loc, 5);
 	isoValueLoc = glGetUniformLocation(lighting->shaderProgram, "isoValue");
 	glUniform1f(isoValueLoc, isoValue);

	prepareColorMap();
	texture1D = new Texture(GL_TEXTURE_1D);
	texture1D->createTexture1D(colorMapData, 33);
}

static void setOFFBasedPropertiesCPU(OFF_CPU* offFile){
	extremeMax = offFile->extremeMax;
	numVertices = offFile->numVertices;
	numIndices = offFile->numIndices;
	numFaces = offFile->numFaces;
	meanProteinPosition = offFile->meanProteinPosition;
}

static void setOFFBasedPropertiesGPU(OFF_GPU* offFile){
	extremeMax = offFile->extremeMax;
	numVertices = offFile->numVertices;
	numIndices = offFile->numIndices;
	numFaces = offFile->numFaces;
	meanProteinPosition = offFile->meanProteinPosition;
}

static void loadScalarField(){

	field = loadField(potFilePath.c_str());
	printf("Scalar Field\n");
    printf("Dimensions: %d %d %d\n", field->dim[0], field->dim[1], field->dim[2]);
    printf("Origin: %f %f %f\n", field->origin[0], field->origin[1], field->origin[2]);
    printf("Step: %f %f %f\n", field->step[0], field->step[1], field->step[2]);
    printf("Min: %f Max: %f\n", field->vmin, field->vmax);
}

OFF_CPU* initializeOffFileCPU(){

	OFF_CPU* offFile = new OFF_CPU(offFilePath);
	loadScalarField();	

	offFile->readOffFile();
	offFile->computeNormals();
	offFile->computeScalarFieldValue(field);
	offFile->getIsoContourPoints(isoValue);
	isoCountourSize = offFile->isoContourPoints.size();
	setOFFBasedPropertiesCPU(offFile);
	return offFile;
}

OFF_GPU* initializeOffFileGPU(){

	OFF_GPU* offFile = new OFF_GPU(offFilePath);
	loadScalarField();
	initTextures();	

	offFile->readOffFile();
	offFile->computeNormals();
	offFile->mapTexCoordinates(field);
	setOFFBasedPropertiesGPU(offFile);
	return offFile;
}

static void initLighting(char cg){
	lighting = new Lighting(cg);
	light.lightDirection = (Vector3f{1.0, 1.0, -1.0}).Normalize();
	light.lightColor = Vector3f(1, 1, 1);
	light.diffuseLightIntensity = 0.75;
	light.ambientLightIntensity = 0.4;
}

static void setGLProperties(){

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);	 
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	std::cout << "Done setting gl props" << std::endl;
}

static void onInitCPU(int argc, char * argv[]){
	
	initLighting('C');
	offFile = initializeOffFileCPU();
	
	createProteinVertexBufferCPU(offFile);
	createIsoVertexBuffer(offFile);
	createIndexBufferCPU(offFile);

	setGLProperties();
}

static void onInitGPU(int argc, char * argv[]){

	initLighting('G');
	OFF_GPU* offFile = initializeOffFileGPU();
	
	createProteinVertexBufferGPU(offFile);
	createIndexBufferGPU(offFile);

	setGLProperties();
}

static void onVisible(int state) {
	if (state == GLUT_VISIBLE) {
		/* tell glut to show model again */
		glutIdleFunc(onIdle);
	} else {
		glutIdleFunc(NULL);
	}
}

static void onKeyPress(unsigned char key, int x, int y) {
	switch (key) {
		case 's':
			viewAngle+=0.3;
			break;
		case 'a':
			viewAngle-=0.3;
			break;
		case 'r':	
			rotationMatrix.InitIdentity();
			previousMatrix.InitIdentity();
			viewAngle = 45.0;
			isoValue = 0.2;
			xDisplace = 0;
			yDisplace = 0;
			break;
		case 'w':
			isoValue+=0.01;
			if(mode == 'C'){
				offFile->getIsoContourPoints(isoValue);
				createIsoVertexBuffer(offFile);	
			} else glUniform1f(isoValueLoc, isoValue);
			break;
		case 'e':
			isoValue-=0.01;
			if(mode == 'C'){
				offFile->getIsoContourPoints(isoValue);
				createIsoVertexBuffer(offFile);
			} else glUniform1f(isoValueLoc, isoValue);	
			break;
		case 'i':
			yDisplace+=0.005;
			break;
		case 'j':
			xDisplace-=0.005;
			break;
		case 'k':
			yDisplace-=0.005;
			break;
		case 'l':
			xDisplace+=0.005;
			break;				
		case 'Q':
		case 'q':
		case 27:
			exit(0);
	}

	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

static void initializeGlutCallbacks(){
	glutKeyboardFunc(onKeyPress);
	glutVisibilityFunc(onVisible);
	glutMotionFunc(onMouseMotion);
	glutMouseFunc(onMouseButtonPress);
	glutPassiveMotionFunc(onMousePassiveMotion);
}

/* -------------------------- Main Here --------------------------------- */

int main(int argc, char* argv[]){
	
	if(argc != 4){
		std::cout << "Usage : " << argv[0] << "  <G for GPU/C for CPU>  <OFFFileName>  <POTFileName>" << std::endl; 
		exit(0);
	}

	mode = *argv[1];
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(title.c_str());
    initializeGlutCallbacks();

    GLenum res = glewInit();
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }

	offFilePath = std::string(argv[2]);
	potFilePath = std::string(argv[3]);

	if(*(argv[1]) == 'G'){
		std::cout << "GPU Based Implementation" << std::endl;
		glutDisplayFunc(renderCallbackGPU);
		glutIdleFunc(renderCallbackGPU);
		onInitGPU(argc, argv);
	}
	else if(*(argv[1]) == 'C'){
		std::cout << "CPU Based Implementation" << std::endl;
		glutDisplayFunc(renderCallbackCPU);
		glutIdleFunc(renderCallbackCPU);
		onInitCPU(argc, argv);
	} else {
		std::cout << "Usage : " << argv[0] << "<G for GPU/C for CPU> <OFFFileName> <POTFileName>"; 
		exit(0);
	}

    glutMainLoop();

	return 0;
}

