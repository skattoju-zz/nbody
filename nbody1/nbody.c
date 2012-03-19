/* COMP 426 Assignment 1
*
* The following is an N-Body simulation that runs on the PPE of the Cell B.E.
* 100 particles are placed randomly in four quadrants of 100 unit cube.
* Forces (Due to gravity) between them are computed and their position velocity
* and acceleration are updated at each iteration.
*
* Siddhartha Kattoju
* 9209905
*
* This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <altivec.h>
#include <time.h>
#include <ppu_intrinsics.h>

#define NO_OF_PARTICLES 1000	/* number of particles (multiple of 4) */
#define INIT_BOUNDING_BOX 100	/* initial positions are bounded */
#define MASS_OF_PARTICLES 1.0	/* mass */
#define EPS2 2000 		/* softening factor */
#define COMPUTE_ITERATIONS 100 	/* no of iterations to compute*/
#define TIME_STEP 0.1 		/* time step */

#define EPS2_VECTOR (__vector float){EPS2,EPS2,EPS2,EPS2}
#define VECINTZERO (__vector int){0,0,0,0}
#define VECZERO (__vector float){0.0,0.0,0.0,0.0}
#define VECONE (__vector float){1.0,1.0,1.0,1.0}
#define VEC3ZERO (__vector float){0.0,0.0,0.0,1.0}
#define VECHALF (__vector float){0.5, 0.5, 0.5, 0.5}
#define MASS_VECTOR (__vector float){MASS_OF_PARTICLES,MASS_OF_PARTICLES,MASS_OF_PARTICLES,MASS_OF_PARTICLES}
#define TIME_STEP_VECTOR (__vector float){TIME_STEP,TIME_STEP,TIME_STEP,TIME_STEP}
#define TIME_SQUARED vec_madd(TIME_STEP_VECTOR,TIME_STEP_VECTOR,VECZERO)

/* particle struct */
typedef struct
{
 __vector float position;
 __vector float velocity;
 __vector float acceleration;
 __vector float mass; //padding in order to byte align 64

}particle;

/* util functions */
void debug_print_float_vector(__vector float var, char* name){

 float *vec = (float*)&var;
 printf("%s : %f %f %f %f \n", name, vec[0], vec[1], vec[2], vec[3]);

}

void debug_print_int_vector(__vector int var, char* name){

 int *vec = (int*)&var;
 printf("%s : %d %d %d %d \n", name, vec[0], vec[1], vec[2], vec[3]);

}

void init_particles(particle* system){

/*
* particles will be placed randomly 4 quadrants of 3d space
* q1: x>0, y>0
* q2: x<0, y>0
* q3: x<0, y<0
* q4: x>0, y<0
*
*/

 int i = 0;
 srand(time(NULL));

 // q1 x>0, y>0
 for(; i < NO_OF_PARTICLES/4; i++){
   system[i].position = (__vector float){rand()%INIT_BOUNDING_BOX,
rand()%INIT_BOUNDING_BOX,
(rand()%INIT_BOUNDING_BOX*2)-INIT_BOUNDING_BOX, 0};
   system[i].velocity = VECZERO;
   system[i].acceleration = VECZERO;
 }

 // q2 x<0, y>0
 for(; i < NO_OF_PARTICLES/2; i++){
   system[i].position = (__vector float){-rand()%INIT_BOUNDING_BOX,
rand()%INIT_BOUNDING_BOX,
(rand()%INIT_BOUNDING_BOX*2)-INIT_BOUNDING_BOX, 0};
   system[i].velocity = VECZERO;
   system[i].acceleration = VECZERO;
 }

 // q3 x<0, y<0
 for(; i < 3*NO_OF_PARTICLES/4; i++){

   system[i].position = (__vector float){-rand()%INIT_BOUNDING_BOX,
-rand()%INIT_BOUNDING_BOX,
(rand()%INIT_BOUNDING_BOX*2)-INIT_BOUNDING_BOX, 0};
   system[i].velocity = VECZERO;
   system[i].acceleration = VECZERO;
 }

 // q4 x>0, y<0
 for(; i < NO_OF_PARTICLES; i++){

   system[i].position = (__vector float){rand()%INIT_BOUNDING_BOX,
-rand()%INIT_BOUNDING_BOX,
(rand()%INIT_BOUNDING_BOX*2)-INIT_BOUNDING_BOX, 0};
   system[i].velocity = VECZERO;
   system[i].acceleration = VECZERO;

 }


}

void compute_interaction(particle* i , particle* j){

 __vector float radius, radius_sqr, s_vector, displ, accel, distSqr,
distSixth, invDistCube;

 /*compute acceleration of particle i*/
 radius = vec_sub(j->position,i->position);
 radius_sqr = vec_madd(radius,radius, VECZERO);
 distSqr = vec_add(vec_splat(radius_sqr,0),vec_splat(radius_sqr,1));
 distSqr = vec_add(vec_splat(radius_sqr,2),distSqr);
 distSqr = vec_add(EPS2_VECTOR,distSqr);
 distSixth = vec_madd(distSqr,distSqr,VECZERO);
 distSixth = vec_madd(distSixth,distSqr,VEC3ZERO);
 invDistCube = vec_rsqrte(distSixth);
 s_vector = vec_madd(MASS_VECTOR,invDistCube,VECZERO);
 i->acceleration = vec_madd(radius,s_vector,i->acceleration);

 /*compute new position & velocity of particle i*/
 displ = vec_madd(i->velocity,TIME_STEP_VECTOR,i->position);
 accel = vec_madd(VECHALF,i->acceleration, VECZERO);
 i->position = vec_madd(accel,TIME_SQUARED, displ);
 i->velocity = vec_madd(i->acceleration,TIME_STEP_VECTOR, i->velocity);

}


void update_particles(particle* system){

 int i, j;

 //Thread 1 ?
 for(i = 0;i<NO_OF_PARTICLES/4;i++){
   for(j = 0;j<NO_OF_PARTICLES;j++){
     compute_interaction(&system[i],&system[j]);
   }
 }

 //Thread 2 ?
 for(i = NO_OF_PARTICLES/4;i<NO_OF_PARTICLES/2;i++){
   for(j = 0;j<NO_OF_PARTICLES;j++){
     compute_interaction(&system[i],&system[j]);
   }
 }

 //Thread 3 ?
 for(i = NO_OF_PARTICLES/2;i<3*NO_OF_PARTICLES/4;i++){
   for(j = 0;j<NO_OF_PARTICLES;j++){
     compute_interaction(&system[i],&system[j]);
   }
 }

 //Thread 4 ?
 for(i = 3*NO_OF_PARTICLES/4;i<NO_OF_PARTICLES;i++){
   for(j = 0;j<NO_OF_PARTICLES;j++){
     compute_interaction(&system[i],&system[j]);
   }
 }

}

__vector int get_quadrant_count(particle* system){
 
  __vector int quad_count = VECINTZERO;
  __vector int quad_mask = VECINTZERO;
 int i;
 for(i = 0; i < NO_OF_PARTICLES ; i++){

         __vector int top2 = (__vector int){1,1,0,0};
         __vector int bottom2 = (__vector int){0,0,1,1};
         __vector int left2 = (__vector int){0,1,0,1};
         __vector int right2 = (__vector int){1,0,1,0};
	 __vector int mask1, mask2;
   
         __vector float vx = vec_splat(system[i].position,0);
         __vector float vy = vec_splat(system[i].position,1);
	 
	 mask1 = vec_sel(right2, left2, vec_cmpgt(vx,VECZERO));
	 mask2 = vec_sel(top2, bottom2, vec_cmpgt(vy,VECZERO));
         quad_mask = vec_and(mask1,mask2);
	 quad_count = vec_add(quad_count,quad_mask);
	 
 }
 return quad_count;
}

void render(particle* system){

 int i = 0;
 for(; i < NO_OF_PARTICLES; i++){

   float *pos = (float*) &system[i].position;
   float *vel = (float*) &system[i].velocity;
   float *acc = (float*) &system[i].acceleration;

   printf("position : %f %f %f ", pos[0], pos[1], pos[2]);
   printf("velocity : %f %f %f ", vel[0], vel[1], vel[2]);
   printf("acceleration : %f %f %f \n", acc[0], acc[1], acc[2]);

 }
}

int main ()
{

 /* create particle system --> array of particles */
 particle particle_system[NO_OF_PARTICLES] __attribute__((aligned(64)));
 
 /* vector that tracks the no. of particles in each quadrant */
 __vector int quad_count;
 int * qc;
 
 /* place particles in 4 quadrants */
 init_particles(particle_system);

 /* run simulation */
 float simulationTime = 0.0;
 int iterations = COMPUTE_ITERATIONS;
 printf("----------------------------------------------");
 printf("----------------------------------------------\n");
 printf("Running Simulation with %d particles & %d iterations with %f seconds time steps\n",
	NO_OF_PARTICLES, COMPUTE_ITERATIONS, TIME_STEP);
 printf("----------------------------------------------");
 printf("----------------------------------------------\n");

 while(iterations > 0){

   /* Compute */
   update_particles(particle_system);

   /* Display */
   //render(particle_system);

   /* Update Time */
   simulationTime = simulationTime + TIME_STEP;
   printf("----------------------------------");
   printf("Simulation Time: %f |",simulationTime);
   quad_count = get_quadrant_count(particle_system);
   qc = (int*)&quad_count;
   printf(" q1:%d q2:%d q3:%d q4:%d",qc[0], qc[1], qc[2], qc[3]);
   printf("----------------------------------\n");

   iterations --;
 }

 return 0;
}
