#include <iostream>
#include <fstream>
#include "neuro.h"
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
   		while (getline (file,line) ){
     			if(line.length()<4){
				continue;
			}
			if(line[0]=='['){
				if(line=="[Result 1/2-1/2]"){
					currScore=0;
				}
				if(line=="[Result 1-0]"){
					currScore=1;
				}
				if(line=="[Result 0-1]"){
					currScore=-1;
				}
				continue;
			}
			else{
				temp=to_string(currScore)+":"+line+"\n";
				fwrite(temp.c_str(),sizeof(char),temp.length(),out);
			}
   		}
   		file.close();
	}
}
void startSelection(const char* file){
}
void startEvaluation(neuro::network* network, const char* path){
}
//system("./carnoaEngine evolve");	
//pgn-extract -Wfen --selectonly 1:100 --output test.f database.pgn
}

