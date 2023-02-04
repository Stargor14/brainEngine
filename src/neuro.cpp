#include "neuro.h"
#include "color.h"
#include "position.h"
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
	n->connections=(u_long*)malloc(0);
	srand(std::chrono::microseconds(std::chrono::system_clock::now().time_since_epoch()).count());//time since epoch as random seed
	for(int i=0;i<inputNeurons;i++){
		addNeuron(n,0,1);
		addConnection(n,createConnection(i,0,rand()%neuronsInLayer,1));//connects each input neuron to a random neuron in layer 1
	}
	for(int layer=1;layer<layers;layer++){//initializing arrays
		for(int i=0;i<neuronsInLayer;i++){
			addNeuron(n,layer,1);
			if(layer>1){
				addConnection(n,createConnection(i,layer-1,i,layer));
			}
		}
	}
	for(;preMutationAmount>0;preMutationAmount--){
		mutate(n);
	}
	return n;
}
network* init(network* original,bool blank){
	network* n=(network*)malloc(sizeof(network));
	n->neuronMutationRate=original->neuronMutationRate;
	n->connectionMutationRate=original->connectionMutationRate;
	n->neuronMutationStrength=original->neuronMutationStrength;
	n->paramMutationRate=original->paramMutationRate;
	n->layers=original->layers;
	if(!blank){
		n->neuronNum=original->neuronNum;
		n->connectionNum=original->connectionNum;
	}
	else{
		n->neuronNum=0;
		n->connectionNum=0;
	}
	//malloc and then memcpy every pointer in the struct
	n->layerNeuronCount=(int*)malloc(sizeof(int)*n->layers);
	n->neuronWeights=(float*)malloc(sizeof(float)*n->neuronNum);
	n->neuronValues=(float*)malloc(sizeof(float)*n->neuronNum);
	n->connections=(u_long*)malloc(sizeof(long)*n->connectionNum);
	if(!blank){
		memcpy((void*)n->layerNeuronCount,original->layerNeuronCount,sizeof(int)*n->layers);
		memcpy((void*)n->neuronWeights,original->neuronWeights,sizeof(float)*n->neuronNum);
		memcpy((void*)n->neuronValues,original->neuronValues,sizeof(float)*n->neuronNum);
		memcpy((void*)n->connections,original->connections,sizeof(long)*n->connectionNum);	
	}
	return n;
}
network* init(std::string file){
	network* n=(network*)malloc(sizeof(network));	
	return n;	
}
u_long createConnection(u_int fromAddress, u_int fromLayer, u_int toAddress, u_int toLayer){
	u_long ans=0;
	ans|=fromAddress;	
	ans<<=4;
	ans|=fromLayer;
	ans<<=28;
	ans|=toAddress;
	ans<<=4;
	ans|=toLayer;
	return ans;		
}
float eval(network* n, Position pos){
	float ans;
	u_long temp;
	std::bitset<64> x(pos.pawns(Color::WHITE));
	std::cout<<x;
	for(int i=0;i<n->connectionNum;i++){
		temp=n->connections[i];
		*neuronAddressLocator(n,temp&0xF,temp>>4&0xFFFFFFF,true)+=*neuronAddressLocator(n,temp>>32&0xF,temp>>36,true)*(*neuronAddressLocator(n,temp&0xF,temp>>4&0xFFFFFFF,false));
	}
	for(int i=0;i<n->layerNeuronCount[n->layers-1];i++){
		ans+=*neuronAddressLocator(n,n->layers-1,i,true);
	}
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
	children[0]=*init(n1,true);
	children[1]=*init(n2,true);
	children[2]=*init(n1,false);
	children[3]=*init(n1,false);
	for(int layer=0;layer<n1->layers;layer++){//only works if both networks have same amount of layers
		layerNeuron=0;
		children[0].layerNeuronCount[layer]=0;
		children[1].layerNeuronCount[layer]=0;
		for(;layerNeuron<fmin(n1->layerNeuronCount[layer],n2->layerNeuronCount[layer]);layerNeuron++){
			if(rand()%2==0){
				//children[0].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
				//children[1].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
				addNeuron(children,layer,n2->neuronWeights[neuron-n2offset]);
				addNeuron(children+1,layer,n1->neuronWeights[neuron-n1offset]);
			}
			else{
				//children[0].neuronWeights[neuron]=n2->neuronWeights[neuron-n2offset];
				addNeuron(children+1,layer,n2->neuronWeights[neuron-n2offset]);
				addNeuron(children,layer,n1->neuronWeights[neuron-n1offset]);
				//children[1].neuronWeights[neuron]=n1->neuronWeights[neuron-n1offset];
			}
			neuron++;
		}
		for(;n1->layerNeuronCount[layer]>layerNeuron;layerNeuron++){
			if(rand()%2==0){
				addNeuron(children,layer,n1->neuronWeights[neuron-n1offset]);
				addNeuron(children+1,layer,n1->neuronWeights[neuron-n1offset]);
				neuron++;
				n2offset++;
			}
		}
		for(;n2->layerNeuronCount[layer]>layerNeuron;layerNeuron++){
			if(rand()%2==0){
				addNeuron(children,layer,n2->neuronWeights[neuron-n2offset]);
				addNeuron(children+1,layer,n2->neuronWeights[neuron-n2offset]);
				neuron++;
				n1offset++;
			}
		}
	}	
	int c=0;	
	for(;c<fmin(n1->connectionNum,n2->connectionNum);c++){
		if(rand()%2==0){
			addConnection(children,n1->connections[c]);
			addConnection(children+1,n2->connections[c]);
		}
		else{
			addConnection(children,n2->connections[c]);
			addConnection(children+1,n1->connections[c]);
		}
	}
	for(;c<n1->connectionNum;c++){
		if(rand()%2==0){
			addConnection(children,n1->connections[c]);
			addConnection(children+1,n1->connections[c]);
		}
	}
	for(;c<n2->connectionNum;c++){	
		if(rand()%2==0){
			addConnection(children,n2->connections[c]);
			addConnection(children+1,n2->connections[c]);
		}
	}
	for(int i=0;i<children->connectionNum;i++){
		if(!validConnection(children,children->connections[i])){
			removeConnection(children,children->connections+i);		
		}
	}
	for(int i=0;i<(children+1)->connectionNum;i++){
		if(!validConnection(children+1,(children+1)->connections[i])){
			removeConnection(children+1,(children+1)->connections+i);		
		}
	}
	//add remaining connections
	//using paramMutationRate because i am too lazy to add another mutation parameter
	mutate(&children[0]);
	mutate(&children[1]);
	int temp=rand()%20;
	for(int i=0;i<temp;i++){
		mutate(&children[2]);
		mutate(&children[3]);
	}	
	return children;
}
bool validConnection(network* n, u_long connection){
	if((connection>>4&0xFFFFFFF)>n->layerNeuronCount[connection&0xF])return false;
	if((connection>>36)>n->layerNeuronCount[(connection>>32)&0xF])return false;		
	return true;
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
	memmove((void*)(temp+1),(void*)(temp),n->neuronValues+n->neuronNum-temp);
	*temp=0;
	temp=neuronAddressLocator(n,layer,n->layerNeuronCount[layer],false);
	memmove((void*)(temp+1),(void*)(temp),n->neuronWeights+n->neuronNum-temp);
	*temp=weight;
	n->layerNeuronCount[layer]++;
}//create similar addConnection function, possibly replacing createConnection
void addConnection(network* n, u_long connection){
	n->connectionNum++;
	n->connections=(u_long*)realloc(n->connections,sizeof(u_long)*n->connectionNum);
	n->connections[n->connectionNum-1]=connection;
}//this also forces a slight change to the initial connection creations
void removeNeuron(network* n, int layer, int relativeAddress){
	n->neuronNum--;
	float* temp=neuronAddressLocator(n,layer,relativeAddress,true);
	memmove((void*)temp,(void*)(temp-1),n->neuronValues+n->neuronNum-temp);
	temp=neuronAddressLocator(n,layer,relativeAddress,false);
	memmove((void*)temp,(void*)(temp-1),n->neuronWeights+n->neuronNum-temp);
	n->neuronWeights=(float*)realloc(n->neuronWeights,sizeof(float)*n->neuronNum);
	n->neuronValues=(float*)realloc(n->neuronValues,sizeof(float)*n->neuronNum);
	n->layerNeuronCount[layer]--;
}
void removeConnection(network* n, u_long* loc){
	memmove((void*)loc,(void*)(loc+1),n->connections+n->connectionNum-loc);	
	n->connectionNum--;
}
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
void printInfo(network* n, bool verbose){
	std::cout<<"layers: "<<n->layers<<'\n';
	for(int i=0;i<n->layers;i++){
		std::cout<<"layer "<<i<<" neurons: "<<n->layerNeuronCount[i]<<'\n';	
		if(verbose){	
		for(int j=0;j<n->layerNeuronCount[i];j++){
			std::cout<<"neuron "<<i<<","<<j<<" Weight: "<<*neuronAddressLocator(n,i,j,false)<<" Value: "<<*neuronAddressLocator(n,i,j,true)<<'\n';
		}
		}
	}

	if(verbose){	
	for(int i=0;i<n->connectionNum;i++){
		std::cout<<"connection "<<i<<" "<<"to_layer: "<<(n->connections[i]&0xF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"to_address: "<<((n->connections[i]>>4)&0xFFFFFFF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"from_layer: "<<((n->connections[i]>>32)&0xF)<<'\n';
		std::cout<<"connection "<<i<<" "<<"from_address: "<<((n->connections[i]>>36))<<'\n'<<'\n';
	}
	}
	std::cout<<"connections: "<<n->connectionNum<<'\n';
}
float similiarity(network* n1,network* n2){
	float ans;
	return ans;
}
void test(){
	network* n1=init(5,50,10,0.5,0.5,1,0.1, 10);
	network* n2=init(5,50,20,0.5,0.5,1,0.1, 10);
	network* children=reproduce(n1,n2);
	printInfo(&children[0],false);
	printInfo(&children[1],false);
	printInfo(&children[2],false);
	printInfo(&children[3],false);
	std::cout<<eval(n1,Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"))<<"\n";
}
}
