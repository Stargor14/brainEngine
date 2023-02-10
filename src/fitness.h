#ifndef FITNESS_H
#define FITNESS_H

#include "neuro.h"
namespace fitness{
//run evolver and engine processes
//store serialized network in file, file name given in command line argument, x.network
//run fitness evaluation, evaluation files passed in, x.eval
//store result in new file, x.result
//
//EVALUATION FILES: long list of: (FEN string,winning side,score) 
//randomly generate various sets to train and test on
//stage 1 fitness evaluation: make stockfish play itself, pick random position, take its score and the final result of the game
//stage 2 fitness evaluation: self play. ~16 network tournament, top 2 reproduce, replace th
}
#endif
