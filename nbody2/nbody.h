#ifndef NBODY_H_
#define NBODY_H_

#define NO_OF_PARTICLES 100 /* number of particles (multiple of 4) */
#define INIT_BOUNDING_BOX 100 /* initial positions are bounded */
#define MASS_OF_PARTICLES 1.0 /* mass */
#define EPS2 2000 /* softening factor */
#define COMPUTE_ITERATIONS 100 /* no of iterations to compute*/
#define TIME_STEP 0.1 /* time step */
#define SPU_THREADS	4

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
typedef struct{
	
 __vector float position;
 __vector float velocity;
 __vector float acceleration;
 __vector float mass; //padding in order to byte align 64

}particle;

#endif /*NBODY_H_*/
