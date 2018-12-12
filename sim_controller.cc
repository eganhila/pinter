#include "sim_dat.h"
#include "integrator.h"
#include "sim_controller.h"
#include "acceleration.h"
#include <random>
#include <iostream>
#include "hdf5.h"
#define FILE "dset.h5"


Particle SimController :: draw_particle(){
    //Chapman initial pop

    /*float x, y, z, H, a0, a1, a, R0;
    float noon_f=1.0, night_f=0.1;

    //setup randoming
    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> uniform_dist(0, 1);

    // 2.213 km/K assuming all mars things
    H = 2.213*pop_t/pop_m;


    x = uniform_dist(e2);
    y = uniform_dist(e2);
    z = uniform_dist(e2);
    */

    Particle p;
     p.state[0] = 1.2*3390;
    p.state[1] = 0;
    p.state[2] = 0;
    p.state[3] = 0;
    p.state[4] = 0;
    p.state[5] = 0;
    return p;
}

void SimController :: run(){
    int cell_idx;
    int p_idx;
    Particle * cell_particles;
    int * all_status;
    float * positions, * velocities;
    hid_t       file_id;   /* file identifier */
    herr_t      status;

    /* Create a new file using default properties. */
    file_id = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Fclose(file_id);

    // Setup data structures that we will
    // keep re-using
    cell_particles = new Particle[N_particles]; 
    all_status = new int[N_particles];
    positions = new float[N_particles*3];
    velocities = new float[N_particles*3];

    // Iterate over all cells
    for (cell_idx=0; cell_idx<sd->dim3; cell_idx++){

        //Evaluate if we want to check this cell
        if (!eval_cell(cell_idx)){continue;}
        

        // Each cell needs N_particles, this is what we're going
        // to parallelize over
        for (p_idx=0; p_idx< N_particles; p_idx++){

            Particle p;
            //draw particle from distribution
            p = draw_particle();
            cell_particles[p_idx]=p;

            //store init particle state
            for (int i=0;i<3;i++){
                positions[p_idx+i*N_particles] = p.state[i];
                velocities[p_idx+i*N_particles] = p.state[i+3];
            }


            //Setup integrator
            Integrator intg(p, 1,100, 0.1);
            intg.set_sd(*sd);
            intg.set_accel(simDat_accel);
            
            //Particle integrated (work done here)
            all_status[p_idx] = intg.integrate();
            
        }

        // Write out cell
        write_cell_data(cell_idx, positions, velocities, all_status);
    } 

    //Delete data structures
    delete[] cell_particles;
    delete[] all_status;
    delete[] positions; 
    delete[] velocities;



}

void SimController :: write_cell_data(int cell_idx, float * positions, float * velocities, int * all_status){
    hid_t       file_id, dataset_id, dataspace_id, group;  /* identifiers */
    hsize_t  dims1D[1], dims2D[2];
    herr_t      status;
    char sidx[20], dset_name[30];

    dims1D[0] = N_particles;
    dims2D[1] = N_particles;
    dims2D[0] = 3;

    sprintf(sidx, "%010d", cell_idx);

    // open existing file
    file_id = H5Fopen(FILE, H5F_ACC_RDWR, H5P_DEFAULT);
    group = H5Gcreate (file_id, sidx, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    //create/close dataspace, dataset, write data, for status
    sprintf(dset_name, "%s/status", sidx);
    dataspace_id = H5Screate_simple(1, dims1D, NULL);
    dataset_id = H5Dcreate2(file_id, dset_name, H5T_STD_I32BE,
                            dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                      all_status);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);


    //create/close dataspace, dataset, write data, for init location
    sprintf(dset_name, "%s/position", sidx);
    dataspace_id = H5Screate_simple(2, dims2D, NULL);
    dataset_id = H5Dcreate2(file_id, dset_name, H5T_STD_I32BE,
                            dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                      positions);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);


    //create/close dataspace, dataset, write data, for init velocity
    sprintf(dset_name, "%s/velocity", sidx);
    dataspace_id = H5Screate_simple(2, dims2D, NULL);
    dataset_id = H5Dcreate2(file_id, dset_name, H5T_STD_I32BE,
                            dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                      velocities);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);


    //close group and file
    status = H5Gclose(group);
    status = H5Fclose(file_id);
}

bool SimController :: eval_cell(int cell_idx){

    //For now we're just going to evaluate a single cell
    if (cell_idx == 0){ return true;}
    
    return false;
}

void SimController :: set_particle_pop(float mass, float charge, float temperature){
    pop_m = mass;
    pop_q = charge;
    pop_T = temperature;

    pop_set=true;
}