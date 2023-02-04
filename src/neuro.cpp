#include "neuro.h"
#include <iostream>
#include <chrono>
#include <string>
#include <cmath>
namespace neuro{

network* init(int layers,int inputNeurons,int neuronsInLayer, float neuronMutationRate,float connectionMutationRate,float neuronMutationStrength,float paramMutationRate, int preMutationAmount){//network generation from scratch 
	/*
	 *	layers, number of layers to create in network, including input and output layer
	 *	inputNeurons, neurons in first layer that actually receive data
	 *	neuronsInLayer, neurons in layers after input layer
	 *	mutationRates, how often network will mutate
	 *	mutationStrength, how much the nodeWeight will change during each mutation
	 *	paramMutationRate, how often the mutation rates will mutate (including itself)
	 *	preMutationAmount, how many times to run mutation function after generation
	 */
	network* n=(network*)malloc(sizeof(network));
	n->neuronMutationRate=neuronMutationRate;
	n->connectionMutationRate=connectionMutationRate;
	n->neuronMutationStrength=neuronMutationStrength;
	n->paramMutationRate=paramMutationRate;
	n->layers=layers;
	n->layerNeuronCount=(int*)malloc(layers*sizeof(int));	
	n->neuronWeights=(float*)malloc(inputNeurons+(layers-1)*neuronsInLayer*sizeof(float));
	n->neuronValues=(float*)malloc(inputNeurons+(layers-1)*neuronsInLayer*sizeof(float));
	n->connectionNum=0;
	n->neuronNum=0;
	n->connections=(long*)malloc(sizeof(long)*((layers-2)*neuronsInLayer+inputNeurons));
	srand(std::chrono::microseconds(std::chrono::system_clock::now().time_since_epoch()).count());//time since epoch as random seed
	for(int i=0;i<inputNeurons;i++){
		addNeuron(n,0,1);
		n->connections[n->connectionNum++]=createConnection(n->neuronNum,0,rand()%neuronsInLayer,1);//connects each input neuron to a random neuron in layer 1
	}
	for(int layer=1;layer<layers;layer++){//initializing arrays
		for(int i=0;i<neuronsInLayer;i++){
			addNeuron(n,layer,1);
			if(layer>1){
				n->connections[n->connectionNum++]=createConnection(i+neuronsInLayer*(layer-1)+inputNeurons,layer-1,i+neuronsInLayer*layer+inputNeurons,layer);
			}
		}
	}
	for(;preMutationAmount>0;preMutationAmount--){
		mutate(n);
	}
	return n;
}
network* init(network* original){
	network* n=(network*)malloc(sizeof(network));
	n->neuronMutationRate=original->neuronMutationRate;
	n->connectionMutationRate=original->connectionMutationRate;
	n->neuronMutationStrength=original->neuronMutationStrength;
	n->paramMutationRate=original->paramMutationRate;
	n->layers=original->layers;
	n->neuronNum=original->neuronNum;
	n->connectionNum=original->connectionNum;
	//malloc and then memcpy every pointer in the struct
	n->layerNeuronCount=(int*)malloc(sizeof(int)*n->layers);
	n->neuronWeights=(float*)malloc(sizeof(float)*n->neuronNum);
	n->neuronValues=(float*)malloc(sizeof(float)*n->neuronNum);
	n->connections=(long*)malloc(sizeof(long)*n->connectionNum);
	memcpy((void*)original->layerNeuronCount,n->layerNeuronCount,sizeof(int)*n->layers);
	memcpy((void*)original->neuronWeights,n->neuronWeights,sizeof(int)*n->neuronNum);
	memcpy((void*)original->neuronValues,n->neuronValues,sizeof(int)*n->neuronNum);
	memcpy((void*)original->connections,n->connections,sizeof(int)*n->connectionNum);	
	return n;
}
network* init(std::string file){
	network* n=(network*)malloc(sizeof(network));	
	return n;	
}
inline long createConnection(int fromAddress, int fromLayer, int toAddress, int toLayer){
	long ans=fromAddress;	
	ans<<=28;
	ans|=fromLayer;
	ans<<=4;
	ans|=toAddress;
	ans<<=28;
	ans|=toLayer;
	return ans;		
}
float eval(network n, Position pos){
	float ans;
	return ans;
}
network* reproduce(network* n1,network* n2){
	/*
	 * takes 2 parent networks and produces 4 children
	 *
	 * child 0 and 1: sexual reproduction
	 * -------------------------------------------------
	 * about half the each parents genes go into the children 
	 * neurons and connections that dont go into one child go into the other
	 *
	 * if there is an excess of neurons leftover in a certain layer of one of the parents,
	 * about half of them will be randomly copied into both children
	 * 
	 * since the neurons are stored in a linear array, offset values have to be introduced 
	 * if the copying was resumed at the same spot on the next layer some neurons from the parent with less neurons would be skipped 
	 * 
	 * after genetic mixing both children are mutated 
	 *
	 * children 2 and 3: asexual reproduction
	 * -------------------------------------------------
	 * heavily mutated copies of the parents
	 * child 2 comes from parent 1
	 * child 3 from parent 2
	 *
	 */
	network* children=(network*)malloc(4*sizeof(network));
	srand(std::chrono::microseconds(std::chrono::system_clock::now().time_since_epoch()).count());//time since epoch as random seed
	int neuron=0,layerNeuron=0,n1offset=0,n2offset=0;
	children[0]=*init(n1);
	children[1]=*init(n2);
	children[2]=*init(n1);
	children[3]=*init(n2);
	for(int layer=1;layer<n1->layers;layer++){//only works if both networks have same amount of layers
		layerNeuron=0;
		children[0].layerNeuronCount[layer]=0;
		children[1].layerNeuronCount[layer]=0;
		for(;layerNeuron<fmin(n1->layerNeuronCount[layer],n1->layerNeuronCount[layer]);layerNeuron++){
			if(rand()%2){
				children[0].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
				children[1].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
			}
			else{
				children[0].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
				children[1].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
			}
			neuron++;
			children[0].layerNeuronCount[layer]++;
			children[1].layerNeuronCount[layer]++;
		}
		for(;n1->layerNeuronCount[layer]>layerNeuron;layerNeuron++){
			if(rand()%2){
				children[0].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
				children[1].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
				children[0].layerNeuronCount[layer]++;
				children[1].layerNeuronCount[layer]++;
				neuron++;
				n2offset++;
			}
		}
		for(;n2->layerNeuronCount[layer]>layerNeuron;layerNeuron++){
			if(rand()%2){
				children[0].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
				children[1].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
				children[0].layerNeuronCount[layer]++;
				children[1].layerNeuronCount[layer]++;
				neuron++;
				n1offset++;
			}
		}
	}		
	for(int c=0;c<fmin(n1->connectionNum,n2->connectionNum);c++){
		if(rand()%2){
			children[0].connections[c]=n1->connections[c];
			children[1].connections[c]=n2->connections[c];
		}
		else{
			children[1].connections[c]=n1->connections[c];
			children[0].connections[c]=n2->connections[c];
		}
	}
	//add remaining connections
	//using paramMutationRate because i am too lazy to add another mutation parameter
	mutate(&children[0]);
	mutate(&children[1]);
	for(int i=0;i<n1->paramMutationRate+n2->paramMutationRate;i++){
		mutate(&children[2]);
		mutate(&children[3]);
	}	
	return children;
}
void mutate(network* n){
	/*inplace mutation function
	 *
	 *using the functions own mutation parameters,  
	 *randomly creates and destroys connections and nodes
	 *possibly mutates its own mutation parameters
	 *after running neuron and connection mutations, does connection cleanup
	 *making sure that each connection points to an actual neuron 
	*/
	//addNeuron(n,1,1);
}
void addNeuron(network* n, int layer, float weight){
	/* increase value of layerNeuronCount and neuronNum
	 * reallocate memory for weights and values
	 * find new pointer of insertion point, memmove up by one
	 * insert new neuron weight and initialize value as 0
	 */
	
	n->neuronNum++;
	n->neuronWeights=(float*)realloc(n->neuronWeights,sizeof(float)*n->neuronNum);
	n->neuronValues=(float*)realloc(n->neuronValues,sizeof(float)*n->neuronNum);
	float* temp=neuronAddressLocator(n,layer,n->layerNeuronCount[layer],true);
	memmove((void*)temp,(void*)(temp+1),n->neuronValues+n->neuronNum-temp);
	*temp=0;
	temp=neuronAddressLocator(n,layer,n->layerNeuronCount[layer],false);
	memmove((void*)temp,(void*)(temp+1),n->neuronWeights+n->neuronNum-temp);
	*temp=weight;
	n->layerNeuronCount[layer]++;
}//create similar addConnection function, possibly replacing createConnection
 //this also forces a slight change to the initial connection creations
float* neuronAddressLocator(network* n, int layer, int relativeAddress, bool value){
	int addressOffset=0;
	for(int l;l<layer;l++){
		for(int ln=0;ln<n->layerNeuronCount[l];ln++){
			addressOffset++;
		}
	}
	addressOffset+=relativeAddress;
	if(value)return n->neuronValues+addressOffset;
	return n->neuronWeights+addressOffset;
}
void printInfo(network* n){
	std::cout<<"layers: "<<n->layers<<'\n';	
	for(int i=0;i<n->layers;i++){
		std::cout<<"layer "<<i<<" neurons: "<<n->layerNeuronCount[i]<<'\n';	
		for(int j=0;j<n->layerNeuronCount[i];j++){
			std::cout<<"neuron "<<i<<","<<j<<" Weight: "<<*neuronAddressLocator(n,i,j,false)<<" Value: "<<*neuronAddressLocator(n,i,j,true)<<'\n';
		}
	}
}
void test(){
	network* n1=init(5,50,10,0.5,0.5,1,0.1, 10);
	//network* n2=init(5,50,10,0.5,0.5,1,0.1, 10);
	//network* children=reproduce(n1,n2);
	printInfo(n1);
}
}
