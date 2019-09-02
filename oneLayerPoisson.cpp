

#include "Spike/Spike.hpp"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <getopt.h>

//#include <boost/filesystem.hpp>
#include<sys/stat.h>

int main (int argc, char *argv[]){


	string experimentName = string("four_layer_test_sim");

	/*
	CHOOSE SIMULATION COMPONENTS
	*/

	//Create an instance of the model
	SpikingModel* model = new SpikingModel();

	//Set up the simulator with a timestep at which the neuron, synapse and STDP properties will be calculated
	float timestep = 0.0001; //this is calculated in seconds, therefore 0.0001 = 0.1ms
	model->SetTimestep(timestep);

	//Choose an input neuron type
	//ImagePoissonInputSpikingNeurons* input_neurons = new ImagePoissonInputSpikingNeurons();
	PoissonInputSpikingNeurons* input_neurons = new PoissonInputSpikingNeurons();
	//Choose a neuron type
	LIFSpikingNeurons* lif_spiking_neurons = new LIFSpikingNeurons();
	//Choose a synapse type
	ConductanceSpikingSynapses* conductance_spiking_synapses = new ConductanceSpikingSynapses();

	//Allocate these components to the simulator
	model->input_spiking_neurons = input_neurons;
	model->spiking_neurons = lif_spiking_neurons;
	model->spiking_synapses = conductance_spiking_synapses;

	/*
	ADD ACTIVITY MONITORS AND PLASTICITY RULES
	*/

	SpikingActivityMonitor* input_spike_monitor = new SpikingActivityMonitor(input_neurons);
	SpikingActivityMonitor* spike_monitor = new SpikingActivityMonitor(lif_spiking_neurons);
	//here in previous model there were also advanced parameter options

	//Allocate them to the simulator
	model->AddActivityMonitor(spike_monitor);
	model->AddActivityMonitor(input_spike_monitor);

	//STDP parameters
	evans_stdp_plasticity_parameters_struct STDP_PARAMS;
	// Below are the decay rates of the variables for learning: Pre/Post synaptic activities C and D (See Ben Evans)
	float decay_term_tau_C = 0.05; //aki_paper = 0.005 // 0.3(In Ben's model, tau_C/tau_D = 0.003/0.005 v 0.015/0.025 v 0.075/0.125, and the first one produces the best result)
	float decay_term_tau_D = 0.05; //aki_paper = 0.005
	float learning_rate_rho = 0.1f; //0.1 is default
	float synaptic_neurotransmitter_concentration_alpha_C = 0.5; //0.5 is default
	//(Perrinet et al, 2001) On one side, we’ll consider a pool of emitters (corresponding to the docked vesicles
	//containing neuro transmitter) quantified by their relative concentration C. 
	//This quantity is triggered by presynaptic spikes but the pool is limited (leading to synaptic depression). 
	float model_parameter_alpha_D = 0.5;
	//(Perrinet et al, 2001) On the other side, we’ll consider a pool of receivers (corresponding to the sensitivity of postsynaptic sites and that appear to be calcium related mechanism) similarly quantified by D. 
	//This quantity is mediated by postsynaptic spikes and may be related to synaptic facilitation. Referred to in Ben Evan's paper as proportion of unblocked NMDA receptors. 

	//Allocate them to the simulator
	STDP_PARAMS.decay_term_tau_C = decay_term_tau_C;
	STDP_PARAMS.decay_term_tau_D = decay_term_tau_D;
	STDP_PARAMS.model_parameter_alpha_D = model_parameter_alpha_D;
	STDP_PARAMS.synaptic_neurotransmitter_concentration_alpha_C = synaptic_neurotransmitter_concentration_alpha_C;
	STDP_PARAMS.learning_rate_rho = learning_rate_rho;
	EvansSTDPPlasticity* evans_stdp = new EvansSTDPPlasticity(conductance_spiking_synapses, lif_spiking_neurons, input_neurons, &STDP_PARAMS);
	model->AddPlasticityRule(evans_stdp);

	/* 
	SETUP PROPERTIES AND CREATE NETWORK
	Note: 
    All Neuron, Synapse and STDP types have associated parameters structures.
    These structures are defined in the header file for that class and allow us to set properties.
    */


	// SETTING UP INPUT NEURONS

	//loading the correct files
	std::string filepath = "../Data/MatlabGaborFilter/";
	std::string filelist = "../Inputs_Gisi_BO/FileList.txt"; 
	string inputs_for_test_name = "Inputs_Gisi_BO";
	float max_FR_of_input_Gabor = 100.0f; // Hz
	//try this line above set_up_rates ***
	// Creating an input neuron parameter structure
	//image_poisson_input_spiking_neuron_parameters_struct * image_poisson_input_spiking_group_params = new image_poisson_input_spiking_neuron_parameters_struct();

	//input_neurons->set_up_rates(filelist.c_str(), "FilterParameters.txt", (filepath+inputs_for_test_name+"/").c_str(), max_FR_of_input_Gabor);
	//this is an instance of the pointer applied to the model, would seem to set the values fine but where do they connect?

	poisson_input_spiking_neuron_parameters_struct* input_neuron_params = new poisson_input_spiking_neuron_parameters_struct();
	input_neuron_params->group_shape[0] = 8;    // x-dimension of the input neuron layer
    input_neuron_params->group_shape[1] = 8;   // y-dimension of the input neuron layer


    int input_layer_ID = model->AddInputNeuronGroup(input_neuron_params);


	//show rate and gabor index
	//printf("\n input neuron params; rate: ");
	//cout << (*image_poisson_input_spiking_group_params).rate;
	//printf("\n gabor index:");
	//cout << (*image_poisson_input_spiking_group_params).gabor_index;

	//show x and y dimensions; bug check
	//the shape of this relates to the image width found in the files
	//printf("\n input neuron params; x-dimension of matrix: ");
	//cout << (*image_poisson_input_spiking_group_params).group_shape[0];
	//printf("\n input neuron params; y-dimension of matrix: ");
	//cout << (*image_poisson_input_spiking_group_params).group_shape[1];
 	
 	//not sure this can be directly added to the model as its value is not an int; therefore no ID
	//input_neurons->AddGroupForEachGaborType(image_poisson_input_spiking_group_params);
	//how is this getting added to the model??
	//int input_layer_ID = ExampleModel->AddInputNeuronGroup(input_neuron_params);

	// SETTING UP NEURON GROUPS
    // Creating an LIF parameter structure for an excitatory neuron population and an inhibitory
    lif_spiking_neuron_parameters_struct * excitatory_population_params = new lif_spiking_neuron_parameters_struct();
    excitatory_population_params->group_shape[0] = 64;
	excitatory_population_params->group_shape[1] = 64;
    excitatory_population_params->resting_potential_v0 = -0.074f;
    excitatory_population_params->threshold_for_action_potential_spike = -0.053f;
	excitatory_population_params->somatic_capacitance_Cm = 500.0*pow(10, -12);
	excitatory_population_params->somatic_leakage_conductance_g0 = 25.0*pow(10, -9);

	lif_spiking_neuron_parameters_struct * inhibitory_population_params = new lif_spiking_neuron_parameters_struct();
    inhibitory_population_params->group_shape[0] = 32;
    inhibitory_population_params->group_shape[1] = 32;
    inhibitory_population_params->resting_potential_v0 = -0.082f;
    inhibitory_population_params->threshold_for_action_potential_spike = -0.053f;
    inhibitory_population_params->somatic_capacitance_Cm = 214.0*pow(10, -12);
    inhibitory_population_params->somatic_leakage_conductance_g0 = 18.0*pow(10, -9);

    // Create populations of excitatory and inhibitory neurons
  	int excitatory_neuron_layer_ID = model->AddNeuronGroup(excitatory_population_params);
  	int inhibitory_neuron_layer_ID = model->AddNeuronGroup(inhibitory_population_params);

  	// SETTING UP SYNAPSES
    // Creating a synapses parameter structure for connections from the input neurons to the excitatory neurons
    conductance_spiking_synapse_parameters_struct* input_to_excitatory_parameters = new conductance_spiking_synapse_parameters_struct();
    input_to_excitatory_parameters->weight_range[0] = 0.5f;   // Create uniform distributions of weights [0.5, 10.0]
    input_to_excitatory_parameters->weight_range[1] = 10.0f;
    input_to_excitatory_parameters->weight_scaling_constant = excitatory_population_params->somatic_leakage_conductance_g0;
    input_to_excitatory_parameters->delay_range[0] = 8*timestep;    // Create uniform distributions of delays [1 timestep, 5 timesteps]
    input_to_excitatory_parameters->delay_range[1] = 8*timestep;

    input_to_excitatory_parameters->connectivity_type = CONNECTIVITY_TYPE_ALL_TO_ALL;

    // Creating a set of synapse parameters for connections from the excitatory neurons to the inhibitory neurons
    conductance_spiking_synapse_parameters_struct * excitatory_to_inhibitory_parameters = new conductance_spiking_synapse_parameters_struct();
    excitatory_to_inhibitory_parameters->weight_range[0] = 10.0f;
    excitatory_to_inhibitory_parameters->weight_range[1] = 10.0f;
    excitatory_to_inhibitory_parameters->weight_scaling_constant = inhibitory_population_params->somatic_leakage_conductance_g0;
    excitatory_to_inhibitory_parameters->delay_range[0] = 5.0*timestep;
    excitatory_to_inhibitory_parameters->delay_range[1] = 3.0f*pow(10, -3);
    excitatory_to_inhibitory_parameters->connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;

      // Creating a set of synapse parameters from the inhibitory neurons to the excitatory neurons
    conductance_spiking_synapse_parameters_struct * inhibitory_to_excitatory_parameters = new conductance_spiking_synapse_parameters_struct();
    inhibitory_to_excitatory_parameters->weight_range[0] = -5.0f;
    inhibitory_to_excitatory_parameters->weight_range[1] = -2.5f;
    inhibitory_to_excitatory_parameters->weight_scaling_constant = excitatory_population_params->somatic_leakage_conductance_g0;
    inhibitory_to_excitatory_parameters->delay_range[0] = 5.0*timestep;
    inhibitory_to_excitatory_parameters->delay_range[1] = 3.0f*pow(10, -3);
    inhibitory_to_excitatory_parameters->connectivity_type = CONNECTIVITY_TYPE_ALL_TO_ALL;

    // CREATING SYNAPSES
    // When creating synapses, the ids of the presynaptic and postsynaptic populations are all that are required
    // Note: Input neuron populations cannot be post-synaptic on any synapse
    model->AddSynapseGroupsForNeuronGroupAndEachInputGroup(excitatory_neuron_layer_ID, input_to_excitatory_parameters);
    model->AddSynapseGroup(input_layer_ID, excitatory_neuron_layer_ID, input_to_excitatory_parameters);
    model->AddSynapseGroup(excitatory_neuron_layer_ID, inhibitory_neuron_layer_ID, excitatory_to_inhibitory_parameters);
    model->AddSynapseGroup(inhibitory_neuron_layer_ID, excitatory_neuron_layer_ID, inhibitory_to_excitatory_parameters);


    /*
      RUN THE SIMULATION
    */

    // The only argument to run is the number of seconds

    model->finalise_model();



    std::string exppath = "/Users/hakunahahannah/Documents/Projects/Spike/Build/output/" + experimentName;
	std::string initpath = "/Users/hakunahahannah/Documents/Projects/Spike/Build/output/" + experimentName + "/initial";
	std::string trainpath = "/Users/hakunahahannah/Documents/Projects/Spike/Build/output/" + experimentName + "/training";

	mkdir(exppath.c_str(), ACCESSPERMS);
	mkdir(initpath.c_str(), ACCESSPERMS);
	mkdir(trainpath.c_str(), ACCESSPERMS);


	float simtime = 50.0f;
	model->run(simtime);

    spike_monitor->save_spikes_as_txt(trainpath + "/");
    input_spike_monitor->save_spikes_as_txt(initpath + "/");
    model->spiking_synapses->save_connectivity_as_txt(trainpath + "/");


	return 0;
}











