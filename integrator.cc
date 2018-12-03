#define simulation 
#include "integrator.h"
#include "sim_dat.h"
#include "interpolate.h"
#include <stddef.h>



void Integrator::evaluate_derivative( 
                     double t, 
                     float dt, 
                     const float * d_in,
                     float * d_out){
    Particle temp;

    for (int i =0; i<6; i++){
        temp.state[i] = particle->state[i] + d_in[i] * dt;}

    for (int i =0; i<3; i++){
        d_out[i] = temp.state[3+i];}
    //acceleration( temp, t+dt, sd,  &d_out[3]);
}


bool Integrator::integrate_step(){

    bool success = true;
    float  d0[6] = {0,0,0,0,0,0},d1[6],d2[6],d3[6], d4[6];//,k;
    //for (int i =0; i<3; i++){
    //  k.d[i] = particle.state[3+i];
    //  k.d[3+i] = acceleration(particle, t, i);
        //}

    evaluate_derivative( t, 0.0f, d0, d1);
    evaluate_derivative( t, dt*0.5f, d1,  d2);
    evaluate_derivative( t, dt*0.5f, d2, d3);
    evaluate_derivative( t, dt, d3,  d4);

    float ddt[6];

    for (int i =0; i<6; i++){
        ddt[i] = 1.0f/6.0f* (d1[i]+2.0f* (d2[i]+d3[i])+d4[i]);
        particle->state[i] = particle->state[i] + ddt[i] * dt;
        }

    return success;
}

