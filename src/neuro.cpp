#include "neuro.h"
#include "color.h"
#include "position.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
namespace neuro{
network* current;
float randf(bool neg){//generates pseudo random number from 0 to 1 or -1 to 1
	if(neg)return ((rand()%1000)/1000.0) * ((rand())%3-1);
	return ((rand()%1000)/1000.0); 
}
void clear(network* n){
	for(int layer=0;layer<n->layers;layer++){
		for(int i=0;i<n->layerNeuronCount[layer];i++){
			*neuronAddressLocator(n,layer,i,true)=0;
		}
	}
}
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
	srand(std::chrono::system_clock::now().time_since_epoch().count());//time since epoch as random seed
	network* n=(network*)malloc(sizeof(network));
	n->neuronMutationRate=neuronMutationRate;
	n->connectionMutationRate=connectionMutationRate;
	n->neuronMutationStrength=neuronMutationStrength;
	n->paramMutationRate=paramMutationRate;
	n->layers=layers;
	n->layerNeuronCount=(int*)malloc(layers*sizeof(int));	
	n->neuronWeights=(float*)malloc(1);
	n->neuronValues=(float*)malloc(1);
	n->connections=(u_long*)malloc(1);
	n->connectionNum=0;
	n->neuronNum=0;
	for(int i=0;i<inputNeurons;i++){
		addNeuron(n,0,1);
		addConnection(n,createConnection(i,0,rand()%neuronsInLayer,1));//connects each input neuron to a random neuron in layer 1
	}
	for(int layer=1;layer<layers;layer++){//initializing arrays
		for(int i=0;i<neuronsInLayer;i++){
			addNeuron(n,layer,randf(true));
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
	}//if blank, network memory is allocated but not assigned with the values of the original network
	 //used for reproduction
	return n;
}
network* init(const char* file){
	network* n=(network*)malloc(sizeof(network));	
	FILE* f;
	f=fopen(file,"rb");
	fread(&n->neuronMutationRate,sizeof(float),1,f);
	fread(&n->connectionMutationRate,sizeof(float),1,f);
	fread(&n->neuronMutationStrength,sizeof(float),1,f);
	fread(&n->paramMutationRate,sizeof(float),1,f);
	fread(&n->layers,sizeof(int),1,f);
	fread(&n->neuronNum,sizeof(int),1,f);
	fread(&n->connectionNum,sizeof(int),1,f);
	n->layerNeuronCount=(int*)malloc(sizeof(int)*n->layers);//allocate memory for the neurons and connections 
	n->neuronWeights=(float*)malloc(sizeof(float)*n->neuronNum);//allocate memory for the neurons and connections 
	n->neuronValues=(float*)malloc(sizeof(float)*n->neuronNum);//allocate memory for the neurons and connections 
	n->connections=(u_long*)malloc(sizeof(long)*n->connectionNum);//allocate memory for the neurons and connections 
	fread(n->layerNeuronCount,sizeof(int),n->layers,f);
	fread(n->neuronWeights,sizeof(float),n->neuronNum,f);
	//neuronValues not required, since they are always initialized at 0
	fread(n->connections,sizeof(long),n->connectionNum,f);
	fclose(f);
	clear(n);//make sure the neuronValues are set at 0, might be redundant?
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
void insertValue(network* n, int* add, Bitboard b){
	std::bitset<64> bs(b);
	for(int i=0;i<64;i++){
		*neuronAddressLocator(n,0,*add+i,true)=bs[i]*(*neuronAddressLocator(n,0,*add+i,false));
	}
	*add+=64;
}
void insertValue(network* n, int* add, bool b){
	*neuronAddressLocator(n,0,*add,true)=(int)b*(*neuronAddressLocator(n,0,*add,false));	
	*add++;
}
float eval(network* n, Position pos){
	/*
	 * Feeds the bitboard positions into the network
	 * add feeding of all bitboards, subtraction of opposite color value
	 *
	 */
	
	float ans=0;
	u_long temp;
	Color side=pos.side_to_move(), opposite=opposite_color(side);
	int counter=0;
	std::vector<int> numInp;  	
	insertValue(n,&counter,pos.pawns(side));
	insertValue(n,&counter,pos.pawns(opposite));
	insertValue(n,&counter,pos.knights(side));
	insertValue(n,&counter,pos.knights(opposite));
	insertValue(n,&counter,pos.rooks(side));
	insertValue(n,&counter,pos.rooks(opposite));
	insertValue(n,&counter,pos.queens(side));
	insertValue(n,&counter,pos.queens(opposite));
	insertValue(n,&counter,pos.bishops(side));
	insertValue(n,&counter,pos.bishops(opposite));
	insertValue(n,&counter,pos.kings(side));
	insertValue(n,&counter,pos.kings(opposite));
	//12*64
	insertValue(n,&counter,pos.can_castle_kingside(side));
	insertValue(n,&counter,pos.can_castle_kingside(opposite));
	insertValue(n,&counter,pos.can_castle_queenside(side));
	insertValue(n,&counter,pos.can_castle_queenside(opposite));
	insertValue(n,&counter,pos.is_mate());
	insertValue(n,&counter,pos.is_draw());
	insertValue(n,&counter,pos.has_mate_threat(side));
	insertValue(n,&counter,pos.has_mate_threat(opposite));
	//8 -> 906 total
	for(int i=0;i<64;i++){
		insertValue(n,&counter,pos.square_is_attacked((Square)i,side));		
		insertValue(n,&counter,pos.square_is_attacked((Square)i,opposite));		
	}
	for(int i=0;i<n->connectionNum;i++){
		temp=n->connections[i];
	//	std::cout<<"to_layer: "<<(n->connections[i]&0xF)<<"to_address: "<<((n->connections[i]>>4)&0xFFFFFFF)<<'\n';
	//	std::cout<<"from_layer: "<<((n->connections[i]>>32)&0xF)<<"from_address: "<<((n->connections[i]>>36))<<'\n'<<'\n';
	
		*neuronAddressLocator(n,temp&0xF,temp>>4&0xFFFFFFF,true)+=*neuronAddressLocator(n,temp>>32&0xF,temp>>36,true)*(*neuronAddressLocator(n,temp&0xF,temp>>4&0xFFFFFFF,false));
	}
	for(int i=0;i<n->layerNeuronCount[n->layers-1];i++){
		ans+=*neuronAddressLocator(n,n->layers-1,i,true);
//		std::cout<<"adding "<<*neuronAddressLocator(n,n->layers-1,i,true)<<'\n';
	}
	std::cout<<"info 0 score: "<<ans<<'\n';
	clear(n);
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
	int neuron=0,layerNeuron=0,n1offset=0,n2offset=0;
	children[0]=*init(n1,true);
	children[1]=*init(n2,true);
	children[2]=*init(n1,false);
	children[3]=*init(n2,false);
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
	mutate(children);
	mutate(children+1);
	int temp=rand()%10+10;//amount of times to mutate child 2 and 3	
	for(int i=0;i<temp;i++){
		mutate(children+2);
		mutate(children+3);
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
	 * neuron mutation param rules: 
	 * 	for every node, run a randf(false) and if the result is < the mutation rate, run the mutation
	 * 	ex: mutation rate of 0.05, about 5% of the time it would mutate
	 * 	this means mutation rate effectively has to be less than 1, anything greater than 1 will just always mutate
	 * neuron mutation rules:
	 * 	for each neuron, if it is selected for mutation, add randf(true)*mutationStrength to the node weight
	 * 	this means mutationStrength can be higher than 1
	 *connection mutation rules:
	 *	for each conneection run randf(), if selected, either duplicate, delete or create random	
	for(layer){
		addNeuron(n,1,1);
	}
	*/
	for(int layer=1;layer<n->layers;layer++){
		for(int add=0;add<n->layerNeuronCount[layer];add++){
			if(randf(false)<n->neuronMutationRate){
				*neuronAddressLocator(n,layer,add,false)=*neuronAddressLocator(n,layer,add,false)+randf(true)*n->neuronMutationStrength;
			}
			if(randf(false)<n->neuronMutationRate){
				removeNeuron(n,layer,add);
			}
			if(randf(false)<n->neuronMutationRate){
				addNeuron(n,layer,randf(true));
			}
		}
	}	
	for(int i=0;i<n->connectionNum;i++){
		if(randf(false)<n->connectionMutationRate){
			//addConnection(n,n->connections[i]);//duplication
		}
		if(randf(false)<n->connectionMutationRate){
			int from_layer=rand()%(n->layers-1),from_address=rand()%n->layerNeuronCount[from_layer],to_layer=rand()%(n->layers-from_layer)+from_layer,to_address=rand()%n->layerNeuronCount[to_layer];
			addConnection(n,createConnection(from_address,from_layer,to_address,to_layer));//new random
		}
		if(randf(false)<n->connectionMutationRate){
			//removeConnection(n,n->connections+i);
		}
		if(!validConnection(n,n->connections[i])){
			removeConnection(n,n->connections+i);
		}
	}
}
void addNeuron(network* n, int layer, float weight){//memory managment could be a little off?
	/* increase value of layerNeuronCount and neuronNum
	 * reallocate memory for weights and values
	 * find new pointer of insertion point, memmove up by one
	 * insert new neuron weight and initialize value as 0
	 */
	
	n->neuronNum++;
	n->neuronWeights=(float*)realloc(n->neuronWeights,sizeof(float)*(n->neuronNum+1));
	n->neuronValues=(float*)realloc(n->neuronValues,sizeof(float)*(n->neuronNum+1));
	float* temp=neuronAddressLocator(n,layer,n->layerNeuronCount[layer],true);
	memmove((void*)(temp+1),(void*)(temp),((n->neuronValues+n->neuronNum)-temp)*sizeof(float));
	*temp=0;
	temp=neuronAddressLocator(n,layer,n->layerNeuronCount[layer],false);
	memmove((void*)(temp+1),(void*)(temp),((n->neuronWeights+n->neuronNum)-temp)*sizeof(float));
	*temp=weight;
	n->layerNeuronCount[layer]++;
}
void addConnection(network* n, u_long connection){
	n->connectionNum++;
	n->connections=(u_long*)realloc(n->connections,sizeof(u_long)*n->connectionNum);
	n->connections[n->connectionNum-1]=connection;
}
void removeNeuron(network* n, int layer, int relativeAddress){//probably buggy, improper memory managment
	n->neuronNum--;
	float* temp=neuronAddressLocator(n,layer,relativeAddress,true);
	memmove((void*)temp,(void*)(temp-1),((n->neuronValues+n->neuronNum)-temp)*sizeof(float));
	temp=neuronAddressLocator(n,layer,relativeAddress,false);
	memmove((void*)temp,(void*)(temp-1),((n->neuronWeights+n->neuronNum)-temp)*sizeof(float));
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
	for(int l=0;l<layer;l++){
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
			std::cout<<"neuron "<<i<<","<<j<<" Weight: "<<round(*neuronAddressLocator(n,i,j,false)*100)/100<<" Value: "<<*neuronAddressLocator(n,i,j,true)<<'\n';
		}
		}
	}

	if(verbose){//no longer needed for now	
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
	//finish eval function, add all bitboards to the input
	//make flipped version of board run through network and subtract scores?
	//make random weight generation functional
	Position startPos=Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	srand(std::chrono::system_clock::now().time_since_epoch().count());//time since epoch as random seed
	network* n1=init(4,904,150,0.01,0.01,2,0.1,5);
	network* n2=init(4,904,50,0.01,0.01,3,0.1,10);
	network* children=reproduce(n1,n2);
	for(int i=0;i<4;i++){
		std::cout<<eval(children+i,startPos)<<"\n";
		clear(children+i);
	}
	printInfo(children+1,false);
}
void serialize(network* n,const char* file){
  	FILE* f;
	f=fopen(file,"wb+");
	fwrite(&n->neuronMutationRate,sizeof(float),1,f);
	fwrite(&n->connectionMutationRate,sizeof(float),1,f);
	fwrite(&n->neuronMutationStrength,sizeof(float),1,f);
	fwrite(&n->paramMutationRate,sizeof(float),1,f);
	fwrite(&n->layers,sizeof(int),1,f);
	fwrite(&n->neuronNum,sizeof(int),1,f);
	fwrite(&n->connectionNum,sizeof(int),1,f);
	fwrite(n->layerNeuronCount,sizeof(int),n->layers,f);
	fwrite(n->neuronWeights,sizeof(float),n->neuronNum,f);
	//neuronValues no required, since they are always initialized at 0
	fwrite(n->connections,sizeof(long),n->connectionNum,f);
	fclose(f);

	//for each bit in a value, bitshift right than &1 the value to get the bit that is going to be copied
	//then copy the last bit (whole number) into a bitset
	//to deserialize, do the same thing but backwards (<<= and |=) 	
	//
}
//write fitness testing function
//using super basic evaluation strategy, evaluate both sides, see who is "winning"
//most accutrate networks pass through
};
