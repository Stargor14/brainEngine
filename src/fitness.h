#ifndef FITNESS_H
#define FITNESS_H

#include "neuro.h"
#include <string>
using namespace neuro;

namespace fitness{

struct evalPos{//already evaluated position
	int result;
	char* fen;
 	float score;//score provided by stockfish
};
//run evolver and engine processes
//store serialized network in file, file name given in command line argument, x.network
//run fitness evaluation, evaluation files passed in, x.eval
//store result in new file, x.result

//EVALUATION FILES: long list of: (FEN string,winning side,score) 
//randomly generate various sets to train and test on
//stage 1 fitness evaluation: make stockfish play itself, pick random position, take its score and the final result of the game
//stage 2 fitness evaluation: self play. ~16 network tournament, top 2 reproduce, replace th

void startEngine(const char*); 
void generateFile(const char* file);//generate custom file for use by network fitness evaluation function
void startSelection(const char* file);//method for selection when a selection file is provided
void startSelection(int networks, const char* file);//overloaded method for brand new selection if no file is provided
void startEvaluation(network* network, const char* path);//evaluates a single network, writes its result to a file
//command line arguments for running uci mode, evalution mode, etc.
}
#endif
