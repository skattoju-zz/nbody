#define STRINGIFY(A) #A

std::string kernel_source = STRINGIFY(
__kernel void part2(__global float4* pos, __global float4* color, __global float4* vel, __global float4* pos_gen, __global float4* vel_gen, float dt)
{
    //get our index in the array
    unsigned int i = get_global_id(0);

    //copy position and velocity for this iteration to a local variable
    //note: if we were doing many more calculations we would want to have opencl
    //copy to a local memory array to speed up memory access (this will be the subject of a later tutorial)
    float4 p = pos[i];
    float4 v = vel[i];

    float4 rad;
    float4 acc;
    float radsqr;
    float radsixth;
    float invradcubed;

    //we've stored the life in the fourth component of our velocity array
    float life = vel[i].w;
    //decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
    life -= dt;
    //if the life is 0 or less we reset the particle's values back to the original values and set life to 1
    if(life <= 0)
    {
        p = pos_gen[i];
        v = vel_gen[i];
        life = 10000.0;    
    }

    //compute acceleration
    int k;
    for(k=0;k<20000;k++){
        //acc = 0.1;
        rad = p - pos[k];
        radsqr = rad.x*rad.x + rad.y*rad.y + rad.z*rad.z + 0.1;
        radsixth = radsqr*radsqr*radsqr;
        invradcubed = rsqrt(radsixth);
        acc += rad*invradcubed*6.673E-11;
    }   
    
    //update position and velocity
    v = acc*dt;

    //update the position with the new velocity
    p = v*dt + 0.5*acc*dt*dt;

    //update life
    v.w = life;

    //update the arrays with our newly computed values
    pos[i] = p;
    vel[i] = v;
    
    //you can manipulate the color based on properties of the system
    //here we adjust the alpha
    color[i].w = life/10000.0;

}
);
