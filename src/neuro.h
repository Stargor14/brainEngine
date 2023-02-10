#ifndef NEURO_H
#define NEURO_H

#include "color.h"
#include "position.h"
namespace neuro{
	struct network{
		float neuronMutationRate,connectionMutationRate,neuronMutationStrength,paramMutationRate;
		int layers,connectionNum,neuronNum;
	       	int* layerNeuronCount;//number of neurons per layer, max 2^28  
		float* neuronWeights;//array of floats for weights and values of each node, maxvalue floatmax
		float* neuronValues;
		u_long* connections;//64 bit data type, 28 bits for id, 4 bits for layer number, 32 for input 32 for output
	};
	extern network* current;	
	u_long createConnection(u_int fromAddress, u_int fromLayer, u_int toAddress, u_int toLayer);
	network* init(int layers,int inputNeurons,int neuronsInLayer, float neuronMutationRate,float connectionMutationRate,float neuronMutationStrength,float paramMutationRate, int preMutationAmount);//network generation from scratch 
	network* init(network* original);//clone network to new pointer
	network* init(std::string file);//load network from file
	std::string serialize(network* n);//writes network data to a file, returns name of file
	float eval(network* n, Position pos);//runs bitboards through network
	network* reproduce(network n1, network n2);//returns 4 children, 2 mutated inverse crossover children, 1 mutated n1, 1 mutated n2  
	void mutate(network* n);//inplace mutation function, uses networks own mutation hyperparameters
	void addNeuron(network* n, int layer, float weight);
	void addConnection(network* n, u_long connection);
	bool validConnection(network* n, u_long connection);
	void removeConnection(network* n, u_long* loc);
	void removeNeuron(network* n, int layer, int relativeAddress);
	float* neuronAddressLocator(network* n, int layer, int relativeAddress, bool value);
	void printInfo(network* n);
	void test();//test all functions	
};
#endif
