#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define min(a,b) (((a)<(b))?(a):(b))

typedef struct scalarField {
    float* data;
    float vmin, vmax;
    int dim[3];
    float origin[3];
    float step[3];
}ScalarField;

int reqIndex[3];
/* This function will return the scalar value corresponding to the given grid point. Please ensure that the indices are within the range of grid size (i.e. 0 <= indices[k] < field->dim[k]) */
float getGridValue(ScalarField *field, int *indices){
    int x = indices[0];
    int y = indices[1];
    int z = indices[2];
    int index = z*field->dim[0]*field->dim[1] + y*field->dim[0] + x;
    return field->data[index];
}

/* This function will return the scalar value corresponding to the grid point closest (using floor) to the given point */
float getValue(ScalarField *field, float *point){
    int indices[3];
    if(point[0] < field->origin[0]) point[0] = field->origin[0];
    if(point[1] < field->origin[1]) point[1] = field->origin[1];
    if(point[2] < field->origin[2]) point[2] = field->origin[2];
    indices[0] = min((int)((point[0] - field->origin[0])/ field->step[0]), field->dim[0]-1);
    indices[1] = min((int)((point[1] - field->origin[1])/ field->step[1]), field->dim[1]-1);
    indices[2] = min((int)((point[2] - field->origin[2])/ field->step[2]), field->dim[2]-1);
    return getGridValue(field, indices);
}

void getNearestIndex(ScalarField *field, float *point){
    if(point[0] < field->origin[0]) point[0] = field->origin[0];
    if(point[1] < field->origin[1]) point[1] = field->origin[1];
    if(point[2] < field->origin[2]) point[2] = field->origin[2];
    reqIndex[0] = min((int)((point[0] - field->origin[0])/ field->step[0]), field->dim[0]-1);
    //int k =(int)((point[0] - field->origin[0])/ field->step[0]);
    //printf("x no. steps : %d\n value : %f", k, field->origin[0]+float(k*field->step[0]));
    reqIndex[1] = min((int)((point[1] - field->origin[1])/ field->step[1]), field->dim[1]-1);
    reqIndex[2] = min((int)((point[2] - field->origin[2])/ field->step[2]), field->dim[2]-1);
}

float getLinearInterpolation(float* values, float* endPoints, float point){
    float diffvalue = values[1] - values[0];
    float alpha = (point - endPoints[0])/(endPoints[1] - endPoints[0]);
    float returnVal = alpha*values[1] + (1 - alpha)*values[0];
    return returnVal;
}

float getBilinearInterpolation(float* values, float* endPoints, float* point){
    float ep1[2], ep2[2], vals1[2], vals2[2], vals3[2];
    ep1[0] = endPoints[0]; vals1[0] = values[0];
    ep1[1] = endPoints[1]; vals1[1] = values[1];
    ep2[0] = endPoints[2]; vals2[0] = values[2];
    ep2[1] = endPoints[3]; vals2[1] = values[3];
    vals3[0] = getLinearInterpolation(vals1, ep1, point[0]);
    vals3[1] = getLinearInterpolation(vals2, ep1, point[0]);
    float returnVal = getLinearInterpolation(vals3, ep2, point[1]);
    return returnVal;
}

float getTrilinearInterpolation(float* values, float endPoints[8][3], float* point){
    float vals1[4], vals2[4], vals3[2];
    float ep1[4], ep2[4], ep3[2];
    float pt1[2];
    float returnVal;
    /*
    for(int z = 0; z < 8; z++){
                printf("%f %f %f\n", endPoints[z][0], endPoints[z][1], endPoints[z][2]);
    }
    */
    for(int i = 0; i< 4; i++){ vals1[i] = values[i]; }//printf("%f\t", vals1[i]); } printf("\n"); 
    for(int i = 0; i< 4; i++){ vals2[i] = values[i+4]; } //printf("%f\t", vals2[i]); } printf("\n");
    
    ep1[0] = endPoints[0][0]; ep1[1] = endPoints[1][0]; ep1[2] = endPoints[0][1]; ep1[3] = endPoints[2][1];
    ep2[0] = endPoints[4][0]; ep2[1] = endPoints[5][0]; ep2[2] = endPoints[4][1]; ep2[3] = endPoints[6][1];
    ep3[0] =  endPoints[0][2]; ep3[1] = endPoints[4][2]; 
    //std::cout << "endPoints" << std::endl;
    //for(int i = 0; i< 4; i++){ printf("%f\t", ep1[i]); } printf("\n");
    //for(int i = 0; i< 4; i++){ printf("%f\t", ep2[i]); } printf("\n");

    //pt1[0] = point[0] - endPoints[0][0]; pt1[1] = point[1] - endPoints[0][1];
    pt1[0] = point[0];
    pt1[1] = point[1];    
    vals3[0] = getBilinearInterpolation(vals1, ep1, pt1); 
    vals3[1] = getBilinearInterpolation(vals2, ep2, pt1);
    //printf("values : %f\t %f\n", vals3[0], vals3[1]);
    returnVal = getLinearInterpolation(vals3, ep3, point[2]);
    
    return returnVal;
}

float getValueTriLinear(ScalarField *field, float *point){
    // Implement your trilinear interpolation code here
    float endPoints[8][3], values[8];
    float baseX, baseY, baseZ;

    getNearestIndex(field, point);

    for (int i = 0; i<3; i++) {endPoints[0][i] = field->origin[i] + (float)(reqIndex[i]*(field->step[i]));}
    
    baseX = endPoints[0][0];
    baseY = endPoints[0][1];
    baseZ = endPoints[0][2];

   // printf("point[%d]\t =  %f\n point[%d]\t =  %f\n point[%d]\t =  %f\n", 0, point[0], 1, point[1], 2, point[2]);
   // printf("baseX\t =  %f\n baseY\t =  %f\n baseZ\t =  %f\n",baseX, baseY,baseZ);   
    int in = 0;    
    for(int z = 0; z < 2; z++){
        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 2; x++){
                endPoints[in][0] = baseX + x*(field->step[0]);
                endPoints[in][1] = baseY + y*(field->step[1]);
                endPoints[in][2] = baseZ + z*(field->step[2]);
                values[in] = getValue(field, endPoints[in]);
                //printf("values[%d]\t =  %1.8f\n", in, values[in]);
                //printf("endPoints[%d]\t =  %f\nendPoints[%d]\t =  %f\nendPoints[%d]\t =  %f\n", in, endPoints[in][0], in, endPoints[in][1], in, endPoints[in][2]);
                in++;
            } 
        }
    }
    float returnVal = getTrilinearInterpolation(values , endPoints, point);
    //std::cout << "Value: " << returnVal << std::endl;
    return returnVal;
}

ScalarField* loadField(const char* filename){
    
    ScalarField* field = (ScalarField*) malloc(sizeof(ScalarField));

    FILE* fp = fopen(filename, "r");
    char line[100];
    char temp[40];
    /* Read the size of the grid */
    fgets(line, 100, fp);
    sscanf(line, "%d %d %d", &field->dim[0], &field->dim[1], &field->dim[2]);
    /* Read the origin of the 3D scalar field */
    fgets(line, 100, fp);
    sscanf(line, "%s %f %f %f", temp, &field->origin[0], &field->origin[1], &field->origin[2]);
    /* Read the step size of the grid */
    fgets(line, 100, fp);
    sscanf(line, "%s %f %f %f", temp, &field->step[0], &field->step[1], &field->step[2]);
    
    /* allocate required data */
    int totalPts = field->dim[0]*field->dim[1]*field->dim[2];
    field->data = (float *) malloc(totalPts * sizeof(float));
    float* tempData = (float *) malloc(totalPts * sizeof(float));
    /* Read the data from file*/
    for(int i=0;i<totalPts;i++){
        float val;
        fscanf(fp, "%f", &val);
        if (val > 0){
                val = log(1+val);
        } else {
                val = -log(1-val);
        }
        if(i==0){
            field->vmin = field->vmax = val;
        }
        if (val < field->vmin){
            field->vmin = val;
        } else if (val > field->vmax){
            field->vmax = val;
        }
        tempData[i] = val;
    }
    fclose(fp);
    /* Flip order */
    int index = 0;
    for (int z=0; z < field->dim[2]; z++){
        for (int y=0; y < field->dim[1]; y++){
            for (int x=0; x < field->dim[0]; x++){
                int i = x*field->dim[1]*field->dim[2] + y*field->dim[2] + z;
                field->data[index] = tempData[i];
                index++;    
            }
        }
    }
    free(tempData);
    return field;
}

int freeScalarField(ScalarField *field){
    if( field == NULL )
        return 0;
    free(field->data);
    free(field);
    return 1;
}