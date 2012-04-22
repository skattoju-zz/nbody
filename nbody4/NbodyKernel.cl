#define STRINGIFY(A) #A
//neat macro trick: https://github.com/enjalot/adventures_in_opencl/blob/master/part1.5/part1.cl

std::string kernel_source = STRINGIFY(

__kernel void bodyInteraction(__global float4* oldPos, __global float4* newPos, __global float4* allVelocities, __global int* numBodies){	
	
	int bodyIndex = get_global_id(0);

	float EPS2 = 0.01;
	float DT = 0.01;

	float4 acceleration = (float4)(0.0f,0.0f,0.0f,0.0f);
	float4 firstBodyPos = oldPos[bodyIndex];
	float4 distance;
	float4 secondBodyPos;

	// find the acceleration
	int i;
	for(i = 0; i < *numBodies;  i++) {
		if(i != bodyIndex) {	
			secondBodyPos = oldPos[i];
			distance = secondBodyPos - firstBodyPos;
			float distSqr = distance.x * distance.x + distance.y * distance.y + distance.z * distance.z + (float)EPS2;
			float distSixth = distSqr * distSqr * distSqr;
			float invDistCube = rsqrt(distSixth);
			float s = secondBodyPos.w * invDistCube;
			acceleration += distance * s;
		}
	}

	float dt = (float)DT;
	//calculate position and velocity
	newPos[bodyIndex] += (dt * allVelocities[bodyIndex] + 0.5 * dt * dt * acceleration);
	allVelocities[bodyIndex] +=  dt * acceleration;
}
);
