/*
* Siddhartha Kattoju
* Comp 426 project
*/

#include "utils.h"
#include <GL/glut.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
using namespace std;

#define POSITION_RANGE 128 // absolute range or random values
#define MASS 1
#define SIGNED_RAND -25 // signed range of random values
#define NEAR_PLANE 10
#define FAR_PLANE 800
#define WIDTH 1200
#define HEIGHT 800
const int NUM_BODIES = 2048;

int counter = 0; // used to show that bodies are exectuted in the CPU and GPU
cl_mem memSpaceCPU[4];
cl_int error = 0; 
cl_command_queue queue_cpu;
cl_kernel bodyInteraction_CPU_kernel;
cl_float4* oldPos = new cl_float4[NUM_BODIES];
cl_float4* newPos = new cl_float4[NUM_BODIES];
cl_float4* allVelocities = new cl_float4[NUM_BODIES];
const int memSize = sizeof(cl_float4) * NUM_BODIES;
const int* numBodies = &NUM_BODIES;
cl_program program_cpu;
cl_device_id device_cpu;
bool isCPU = true;

void initiateBodies() {
	srand(time(NULL));
	for(int i = 0; i < NUM_BODIES ; i++) {	
		oldPos[i].s[0] = (float)((rand() % POSITION_RANGE) + 1 + SIGNED_RAND);
		oldPos[i].s[1] = (float)((rand() % POSITION_RANGE) + 1 + SIGNED_RAND);
		oldPos[i].s[2] = (float)((rand() % POSITION_RANGE) + 1 + SIGNED_RAND);
		oldPos[i].s[3] = (float)((rand() % POSITION_RANGE) + 1 + SIGNED_RAND)*MASS;
		allVelocities[i].s[0] = 0.0;
		allVelocities[i].s[1] = 0.0;
		allVelocities[i].s[2] = 0.0;
		newPos[i].s[0] = 0;
		newPos[i].s[1] = 0;
		newPos[i].s[2] = 0;
		newPos[i].s[3] = 0;
	}
}

void draw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(20, (GLfloat)WIDTH/(GLfloat)HEIGHT, NEAR_PLANE, FAR_PLANE);
	gluLookAt(10,10,10,0,0,0,0.0,1.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glBegin(GL_POINTS);
	
	for(int i = 0; i < NUM_BODIES; i++) {
		glColor4f(0.0, 1.0, 0.0, oldPos[i].s[3]/128);
		glVertex3f(oldPos[i].s[0], oldPos[i].s[1], oldPos[i].s[2]);
	}
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
	glFlush();
}

void printBodies() {
	for(int i = 0; i < NUM_BODIES; i++) {
		printf("Body %d: X: %.4f Y: %.4f Z: %.4f Mass: %.0f\n", i + 1, oldPos[i].s[0], oldPos[i].s[1], oldPos[i].s[2], oldPos[i].s[3]);
	}
}

 void runCPU() {

	// Extracting the CPU kernel
    error = clSetKernelArg(bodyInteraction_CPU_kernel, 0, sizeof(memSpaceCPU[0]), &memSpaceCPU[0]); // old position
	error |= clSetKernelArg(bodyInteraction_CPU_kernel, 1, sizeof(memSpaceCPU[1]), &memSpaceCPU[1]); // new position
	error |= clSetKernelArg(bodyInteraction_CPU_kernel, 2, sizeof(memSpaceCPU[2]), &memSpaceCPU[2]); // velocity
	error |= clSetKernelArg(bodyInteraction_CPU_kernel, 3, sizeof(memSpaceCPU[3]), &memSpaceCPU[3]); // number of bodies
	assert(error == CL_SUCCESS);

	// Launching the kernel	CPU
	const size_t global_ws_cpu = NUM_BODIES; // total number of work items
	const size_t local_ws_cpu = 512; // number of work items/work group
	error = clEnqueueNDRangeKernel(queue_cpu, bodyInteraction_CPU_kernel, 1, NULL, &global_ws_cpu, &local_ws_cpu, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	clFinish(queue_cpu);

	// Reading data
	clEnqueueReadBuffer(queue_cpu, memSpaceCPU[1], CL_TRUE, 0, memSize, newPos, 0, NULL, NULL);
 }

void update() {
	// printBodies();

	// alternates between CPU and GPU for each frame. 
	runCPU();

	// setting the new positions to the position array
	for(int i = 0; i < NUM_BODIES; i++) {
		oldPos[i].s[0] = newPos[i].s[0];
		oldPos[i].s[1] = newPos[i].s[1];		
		oldPos[i].s[2] = newPos[i].s[2];
	}

	glutPostRedisplay();
}

void resizeWindow(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	gluPerspective(90, (GLfloat)width/(GLfloat)height, NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
}

void initiateOpenGL(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(350, 200);
	glutCreateWindow("N-Body Simulation");

	// Callback registrations with the OpenGL env are done here 
	glutDisplayFunc(draw);
	glutReshapeFunc(resizeWindow);
	glutIdleFunc(update);

	glutMainLoop();
}


int main(int argc, char** argv) {
	
	// init
	cl_platform_id platform_cpu;
	cl_context context_cpu;

	// Platform
	platform_cpu = GetIntelOCLPlatform();
	if(platform_cpu == NULL){
        cout << "checking for NVIDIA GPU" << endl;
        platform_cpu = GetNVIDIAOCLPlatform();
        
         // Device
        error = clGetDeviceIDs(platform_cpu, CL_DEVICE_TYPE_GPU, 1, &device_cpu, NULL);
        if (error != CL_SUCCESS) {
        cout << "Error getting device ids: " << error << ":" << oclErrorString(error) << endl;
        exit(error);
        }
    }else{
        // Device
        error = clGetDeviceIDs(platform_cpu, CL_DEVICE_TYPE_CPU, 1, &device_cpu, NULL);
        if (error != CL_SUCCESS) {
            cout << "Error getting device ids: " << error << ":" << oclErrorString(error) << endl;
            exit(error);
        }
    }
    
    // Context
    context_cpu = clCreateContext(0, 1, &device_cpu, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
    cout << "Error creating context: " << error << endl;
    exit(error);
    }
    
    // Queue
    queue_cpu = clCreateCommandQueue(context_cpu, device_cpu, 0, &error);
    if (error != CL_SUCCESS) {
    cout << "Error creating command queue: " << error << oclErrorString(error) << endl;
    exit(error);
    }
    

	cout << "Initializing bodies" << endl;
	initiateBodies();

	memSpaceCPU[0] = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, memSize, oldPos, &error);
	assert(error == CL_SUCCESS);
	memSpaceCPU[1] = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, memSize, newPos, &error);
	assert(error == CL_SUCCESS);
	memSpaceCPU[2] = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, memSize, allVelocities, &error);
	assert(error == CL_SUCCESS);
	memSpaceCPU[3] = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int*), (void*)numBodies, &error); 
	assert(error == CL_SUCCESS);

	#include "NbodyKernel.cl"
	const char* source_cpu = kernel_source.c_str();
	size_t src_size_cpu = kernel_source.size();
    program_cpu = clCreateProgramWithSource(context_cpu, 1, &source_cpu, &src_size_cpu, &error);
    if (error != CL_SUCCESS) {
	   cout << "program creation failed: " << error << endl;
	   exit(error);
	}
    assert(error == CL_SUCCESS);

	// Builds the CPU program
	error = clBuildProgram(program_cpu, 1, &device_cpu, NULL, NULL, NULL);
	if (error != CL_SUCCESS) {
	   cout << "program not built: " << error << endl;
	   //exit(error);
	}
	
	// Show logs
	char* buildLog;
	size_t logSize;
	// First call to know the proper size
	clGetProgramBuildInfo(program_cpu, device_cpu, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	buildLog = new char[logSize + 1];
	// Second call to get the log
	clGetProgramBuildInfo(program_cpu, device_cpu, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
	buildLog[logSize] = '\0';
	cout << "printing build logs" << endl;
	cout  << buildLog << endl;
	delete[] buildLog;
    assert(error == CL_SUCCESS);

	// Extracting the CPU kernel
	bodyInteraction_CPU_kernel = clCreateKernel(program_cpu, "bodyInteraction", &error);
	if (error != CL_SUCCESS) {
	   cout << "Error creating command queue: " << error << oclErrorString(error) << endl;
	   exit(error);
	}

	assert(error == CL_SUCCESS);

	cout << "starting main loop" << endl;

	//start openGL. The update function calls the kernel code
	initiateOpenGL(argc, argv);
	
	// clean up
	delete [] oldPos;
	delete [] newPos;
	delete [] allVelocities;

	clReleaseKernel(bodyInteraction_CPU_kernel);
	clReleaseCommandQueue(queue_cpu);
	clReleaseContext(context_cpu);
	clReleaseMemObject(memSpaceCPU[0]);
	clReleaseMemObject(memSpaceCPU[1]);
	clReleaseMemObject(memSpaceCPU[2]);
	clReleaseMemObject(memSpaceCPU[3]);

	return 0;
}