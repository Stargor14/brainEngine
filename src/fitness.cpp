#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include "neuro.h"
#include "position.h"
#include "search.h"
#include "uci.h"
using namespace std;
namespace fitness{
Move BestMove;
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
	neuro::network* networks=(neuro::network*)malloc(sizeof(neuro::network)*population);
	for(int i=0;i<population;i++){
		networks[i]=*neuro::init(to_string(i).append(".network").c_str());
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
		for(int i=0;i<population;i++){
			cout<<"network"<<networks[i].id<<" score: "<<scores[i]<<'\n';
		}
		//order success rate of networks
		//cout<<"started ordering\n";

		for(int i=0;i<population-1;i++){
			for(int j=0;j<population-1;j++){
				if(scores[j]<scores[j+1]){
					int temp=scores[j];
					neuro::network stemp=networks[j];
					scores[j]=scores[j+1];
					networks[j]=networks[j+1];
					scores[j+1]=temp;
					networks[j+1]=stemp;
				}
			}
		}
		cout<<'\n';
		//rewrite all networks in their new order
		
		neuro::serialize(networks,"best.network");	
		children=neuro::reproduce(networks,networks+1);
		networks[population-4]=children[0];
		networks[population-3]=children[1];
		networks[population-2]=children[2];
		networks[population-1]=children[3];

		for(int i=0;i<population;i++){
			neuro::serialize(networks+i,to_string(i).append(".network").c_str());	
		}
		//free(children);
		gen++;
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
		n=neuro::init(5,neuro::inputNum,500,0.01,0.01,1,0.1,10);
		neuro::serialize(n,to_string(i).append(".network").c_str());
	}
	startSelection(std::string(name).append(".selection").c_str());
}
void testgo(Position RootPosition, int depth) {
    int time[2] = {0, 0}, inc[2] = {0, 0}, movesToGo = 0, nodes = 0;
    int moveTime = 0;
    bool infinite = false, ponder = false;
    Move searchMoves[500];

    searchMoves[0] = MOVE_NONE;
    

    think(RootPosition, infinite, ponder, time[RootPosition.side_to_move()],
          inc[RootPosition.side_to_move()], movesToGo, depth, nodes, moveTime,
          searchMoves);
  }
 
void startGame(const char* path1,const char* path2, int searchDepth, int moveNumberLimit){
	Position currPos=Position(StartPosition);
	neuro::network* networks[2]= {neuro::init(path1),neuro::init(path2)};
	bool currentNetwork=false;//false for network 1, true for 2
	int ply=0;
	//think() the current position, passing in time per move
	//take the best move
	//do_move() on the best move
	//switch neuro::current to the other network
	//END CONDITIONS
	//checkmate
	//draw
	//totalmoves>moveNumberLimit
//	currPos.print();
	while(ply<moveNumberLimit*2&&currPos.is_ok()&&!currPos.is_mate()&&!currPos.is_draw()){
	//	think(currPos,true,true,0,0,0,0,0,timePerMove,moves);
		neuro::current=networks[(int)currentNetwork];
		testgo(currPos,searchDepth);
		cout<<"BESTEST GREATESTMOVE "<<BestMove;
		cout<<'\n';
		currPos.do_move(BestMove);
		currentNetwork=!currentNetwork;
		currPos.print();
		ply++;
	}
	currPos.print();
}
int roundScore(float in){
	if(in>3)return 1;
	if(in<-3)return -1;
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
