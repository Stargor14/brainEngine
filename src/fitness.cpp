#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include "neuro.h"
#include "position.h"
using namespace std;
namespace fitness{
//./pgn-extract -Wfen --selectonly 1:100 --output test.f database.pgn -Rroster --xroster
void generateFile(const char* path){
	string line,temp;
	ifstream file;
  	FILE* out=fopen(string(path).append(".eval").c_str(),"w+");
	file.open(path);
	int currScore=0;
	if(file.is_open()) {
   		while (getline(file,line)){
     			if(line.length()<4){
				continue;
			}
			if(line[0]=='['){
				if(line=="[Result \"1/2-1/2\"]"){
					currScore=0;
				}
				if(line=="[Result \"1-0\"]"){
					currScore=1;
				}
				if(line=="[Result \"0-1\"]"){
					currScore=-1;
				}
				continue;
			}
			else{
				temp=to_string(currScore)+":"+line+"\n";
				fwrite(temp.c_str(),sizeof(char),temp.length(),out);
				std::cout<<temp;
			}
   		}
   		file.close();
	}
}
void startSelection(const char* path){
	int gen,maxGen,population;
	fstream file,t;
	string line,evalFile;
	file.open(path);
	bool waiting=true;
	neuro::network* children,ntemp;
	if(file.is_open()){
		getline(file,line);
		evalFile=line;
		getline(file,line);
		population=stoi(line);
		getline(file,line);
		gen=stoi(line);
		getline(file,line);
		maxGen=stoi(line);
	}
	else return;
	int* scores=(int*)malloc(sizeof(int)*population); 
	int* networks=(int*)malloc(sizeof(int)*population);
	for(int i=0;i<population;i++){
		networks[i]=i;
	}
	cout<<"population: "<<population;
	while(gen<maxGen){
		cout<<"started generation: "<<gen<<"\n";
		for(int i=0;i<population;i++){
			system(std::string("rm ").append(to_string(i)).append(".network.result").c_str());
			system(std::string("./carnosaEngine -e ").append(to_string(i)).append(".network ").append(evalFile).c_str());
			//cout<<"started proccess "<<i<<'\n';
		}
		for(int i=0;i<population;i++){
			waiting=true;
			while(waiting){
				t.open(to_string(i)+".network.result");
				if(t.is_open()){
					waiting=false;
					getline(t,line);
					scores[i]=stoi(line);
					//cout<<scores[i]<<" "<<networks[i]<<".network\n";
				}
				//cout<<"tried to open "<<i<<".network.result\n";
				//std::this_thread::sleep_for(std::chrono::seconds(1));
				t.close();
			}
			//record success rate
		}
		cout<<"\n";
		//order success rate of networks
		//cout<<"started ordering\n";
		for(int i=0;i<population-1;i++){
			for(int j=0;j<population-1;j++){
				if(scores[j]<scores[j+1]){
					int temp=scores[j],stemp=networks[j];
					scores[j]=scores[j+1];
					networks[j]=networks[j+1];
					scores[j+1]=temp;
					networks[j+1]=stemp;
				}
			}
		}	
		for(int i=0;i<population;i++){
			cout<<scores[i]<<" "<<networks[i]<<".network "<<neuro::init(to_string(networks[i]).append(".network").c_str())->connectionNum<<'\n';
		}
		cout<<'\n';
		//rewrite all networks in their new order
		/*
		for(int i=0;i<population;i++){
			neuro::serialize(neuro::init(to_string(networks[i]).append(".network").c_str()),to_string(i).append(".network").c_str());	
		}
		*/
		children=neuro::reproduce(neuro::init(to_string(networks[0]).append(".network").c_str()),neuro::init(to_string(networks[1]).append(".network").c_str()));
		neuro::serialize(children,to_string(networks[population-1]).append(".network").c_str());	
		neuro::serialize(children+1,to_string(networks[population-2]).append(".network").c_str());	
		neuro::serialize(children+2,to_string(networks[population-3]).append(".network").c_str());	
		neuro::serialize(children+3,to_string(networks[population-4]).append(".network").c_str());	
		//free(children);
		gen++;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}	
}
void startSelection(int networks,int generations,const char* name,const char* file){
	neuro::network* n;
	string temp;
  	FILE* out=fopen(std::string(name).append(".selection").c_str(),"w+");
	fwrite(file,sizeof(char),std::string(file).size(),out);//population size
	fwrite("\n",sizeof(char),1,out);
	fwrite(std::to_string(networks).c_str(),sizeof(char),to_string(networks).size(),out);//population size
	fwrite("\n",sizeof(char),1,out);
	fwrite(std::to_string(0).c_str(),sizeof(char),to_string(0).size(),out);//population size
	fwrite("\n",sizeof(char),1,out);
	fwrite(std::to_string(generations).c_str(),sizeof(char),to_string(generations).size(),out);//population size
	fclose(out);
	for(int i=0;i<networks;i++){
		n=neuro::init(5,neuro::inputNum,800,0.03,0.03,3,0.1,20);
		neuro::serialize(n,to_string(i).append(".network").c_str());
	}
	startSelection(std::string(name).append(".selection").c_str());
}
int roundScore(float in){
	if(in>0)return 1;
	if(in<0)return -1;
	return 0;
}
void startEvaluation(const char* netPath,const char* evalPath){
	neuro::network* n=neuro::init(netPath);
	string line;
	ifstream file;
  	FILE* out=fopen(string(netPath).append(".result").c_str(),"w+");
	file.open(evalPath);
	int correct=0,total=0,result;
	if(file.is_open()){
   		while(getline(file,line)){
			result=neuro::eval(n,line.substr(line.find(":")+1));
			if(roundScore(result)==stoi(line.substr(0,line.find(":"))))correct++;	
			total++;
   		}
   		file.close();
	}
	fwrite(std::to_string(correct).c_str(),sizeof(char),to_string(correct).size(),out);
	fwrite("\n",sizeof(char),1,out);
	fwrite(std::to_string(total).c_str(),sizeof(char),to_string(total).size(),out);
	fwrite("\n",sizeof(char),1,out);
	fclose(out);
}	
//pgn-extract -Wfen --selectonly 1:100 --output test database.pgn
}
