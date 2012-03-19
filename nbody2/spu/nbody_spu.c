#include <stdio.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include "nbody.h"

/* util macros */
#define mfc_ea2l(ea)   (unsigned int)(ea)
#define mfc_ea2h(ea)   (unsigned int)((unsigned long long)(ea)>>32)
#define spu_writech(imm, ra) si_wrch((imm), si_from_uint(ra))

typedef struct{

	particle* system;
	int start_index;
	int end_index;
	int tag;
	unsigned long long ctx;

}spu_args_t;

void compute_interaction(particle* i , particle* j){

	 __vector float radius, radius_sqr, s_vector, displ, accel, distSqr,
	distSixth, invDistCube, timesquared, radius_sqr0, radius_sqr1, radius_sqr2;
	
	 /*compute acceleration of particle i*/
	 radius = spu_sub(j->position,i->position);
	 radius_sqr = spu_madd(radius,radius, VECZERO);
	 radius_sqr0 = spu_splats(radius_sqr[0]);
	 radius_sqr1 = spu_splats(radius_sqr[1]);
	 radius_sqr2 = spu_splats(radius_sqr[2]);
	 distSqr = spu_add(radius_sqr0, radius_sqr1);
	 distSqr = spu_add(radius_sqr2,distSqr);
	 distSqr = spu_add(EPS2_VECTOR,distSqr);
	 distSixth = spu_madd(distSqr,distSqr,VECZERO);
	 distSixth = spu_madd(distSixth,distSqr,VEC3ZERO);
	 invDistCube = spu_rsqrte(distSixth);
	 s_vector = spu_madd(MASS_VECTOR,invDistCube,VECZERO);
	 i->acceleration = spu_madd(radius,s_vector,i->acceleration);
	
	 /*compute new position & velocity of particle i*/
	 displ = spu_madd(i->velocity,TIME_STEP_VECTOR,i->position);
	 accel = spu_madd(VECHALF,i->acceleration, VECZERO);
	 timesquared = spu_madd(TIME_STEP_VECTOR,TIME_STEP_VECTOR,VECZERO);
	 i->position = spu_madd(accel,timesquared, displ);
	 i->velocity = spu_madd(i->acceleration,TIME_STEP_VECTOR, i->velocity);

}

int main(spu_args_t spu_args){
	
	  printf("SPE%d: allocating memory in local store \n",spu_args.tag);

	 /* create particle system structure --> array of particles */
	 particle particle_system[NO_OF_PARTICLES] __attribute__((aligned(64)));
	 
	  printf("SPE%d: dma data in \n",spu_args.tag);
	
	/* dma stuff into the structure */
	spu_mfcdma64(particle_system, mfc_ea2h(spu_args.system), mfc_ea2l(spu_args.system), sizeof(particle)*(spu_args.end_index-spu_args.start_index), spu_args.tag, MFC_GET_CMD);
	
	  printf("SPE%d: wait for dma \n",spu_args.tag);

	spu_writech(MFC_WrTagMask, 1 << spu_args.tag);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
	
	  printf("SPE%d: computer interactions \n",spu_args.tag);
	
	/* compute */
	int i , j;
	for(i = spu_args.start_index;i<spu_args.end_index;i++){
		for(j = 0;j<NO_OF_PARTICLES;j++){
			compute_interaction(&spu_args.system[i],&spu_args.system[j]);
		}
	}
	
	  printf("SPE%d: dma data out \n",spu_args.tag);
	
	/* dma stuff out */
	spu_mfcdma64(particle_system, mfc_ea2h(spu_args.system), mfc_ea2l(spu_args.system), sizeof(particle)*NO_OF_PARTICLES, spu_args.tag, MFC_PUT_CMD);
	spu_writech(MFC_WrTagMask, 1 << spu_args.tag);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
	
	return 0;
}
