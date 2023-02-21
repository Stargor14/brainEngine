/*
  Glaurung, a UCI chess playing engine.
  Copyright (C) 2004-2008 Tord Romstad

  Glaurung is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Glaurung is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


////
//// Includes
////

#include <cstdlib>
#include <iostream>

#include "benchmark.h"
#include "bitboard.h"
#include "direction.h"
#include "endgame.h"
#include "evaluate.h"
#include "material.h"
#include "mersenne.h"
#include "misc.h"
#include "movepick.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "uci.h"
#include "ucioption.h"
#include "fitness.h"
#include "neuro.h"

//// 
//// Functions
////
int main(int argc, char *argv[]) {

  // Disable IO buffering
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  std::cout.rdbuf()->pubsetbuf(NULL, 0);
  std::cin.rdbuf()->pubsetbuf(NULL, 0);

  // Initialization

  init_mersenne();
  init_direction_table();
  init_bitboards();
  init_uci_options();
  Position::init_zobrist();
  Position::init_piece_square_tables();
  MaterialInfo::init();
  MovePicker::init_phase_table();
  init_eval(1);
  init_bitbases();
  init_threads();

  //neuro::test();

  // Make random number generation less deterministic, for book moves
  int i = abs(get_system_time() % 10000);
  for(int j = 0; j < i; j++)
    genrand_int32();


  // Print copyright notice

  //selecting mode to run in with command linearguments
  if(argc>1){
 	if(!strcmp(argv[1],"evaluate")||!strcmp(argv[1],"-e")){//start fitness evaluation of a single network
		if(argc>3){
			if(std::string(argv[2]).find(".network") != std::string::npos&&std::string(argv[3]).find(".eval") != std::string::npos){//providing .selection file of network files for continued selection
				fitness::startEvaluation(argv[2],argv[3]);//third arguemnt path to evaluation file
			}
		}
		else if(argc==3){
			std::cout<<"please provide a valid positional info file\n";//if no eval file or network file provided
		}
	}
	else if(std::string(argv[1]).find("selection")!=std::string::npos|| std::string(argv[1]).find("sof")!=std::string::npos||std::string(argv[1]).find("-s")!=std::string::npos){
		if(std::string(argv[2]).find(".selection") != std::string::npos){
			fitness::startSelection(argv[2]);//resume selection from .selection file	
		}
		else if(std::string(argv[3]).find(".eval") != std::string::npos){
			fitness::startSelection(atoi(argv[4]),atoi(argv[5]),argv[2],argv[3]);//start a new selection from scratch, using provided evaluation file	
		}
		else{
			std::cout<<"please provide a selection file, or name for new selection file, name of eval file, the number of networks in each generation, and the number of generations to run selection for\n";
		}
	}
	else if(!strcmp(argv[1],"uci")||!strcmp(argv[1],"-u")){
		if(std::string(argv[2]).find(".network") != std::string::npos){
			//add uci mode?
		}
	}
	else if(!strcmp(argv[1],"generate")||!strcmp(argv[1],"-g")){
		fitness::generateFile(argv[2]);	
	}
	else if(std::string(argv[1]).find("info")!=std::string::npos){
		
		neuro::printInfo(neuro::init(argv[2]),true);

	}
	else{
		std::cout<<"no valid argument provided, run with -h or help\n";
	}
  }
  else{//if no argument is provided
  	std::cout << engine_name() << " by Eryk Halicki, based on Glaurung 2.2"<< std::endl;
//  	neuro::current=neuro::init(4,904,150,0.01,0.01,2,0.1,5);//instead of blank network, use a specific .network file
	neuro::current=neuro::init("best.network");
  	uci_main_loop();
  }
  return 0;
}
