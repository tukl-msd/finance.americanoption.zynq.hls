/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier A. Varela (varela@eit.uni-kl.de)
//
// Last review: 29/06/2015
//
// Using: Xilinx Vivado HLS 2014.3
//
///////////////////////////////////////////////////  

#include "mt_flex_hls.hpp"

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <iostream>

#include "mt19937ar.h"

#define NRND 1000//500//(625*2)

int main(int argc, char *argv[]) 
{
	hls::stream<uint32_t> hw_rnd;
	hls::stream<uint32_t> sw_rnd;
	uint32_t sw_rndVector[NRND];

	// generate seeds
	unsigned int seeds [MT_N];
	unsigned int states[MT_N];

	init_genrand(0);
	generate_numbers();
	get_mt_state(seeds);

	std::cout << "---------------------------------------------"<< std::endl;
	std::cout << "get_mt_state(seeds) has finished" << std::endl;

	std::cout << "---------------------------------------------"<< std::endl;
	std::cout << "running mersenne_twister (SW) ..." << std::endl;


	for (int i = 0; i < NRND; ++i)
	{
		uint32_t temp = genrand_int32();
		sw_rndVector[i] = temp;
		sw_rnd.write(temp);
	}
	for (int i = NRND-1; i>=0; --i)
	{
		sw_rnd.write(sw_rndVector[i]);
	}

	
	std::cout << "---------------------------------------------"<< std::endl;
	std::cout << "running mersenne_twister (HW) ..." << std::endl;

	volatile uint32_t peekAmountFwd;
	volatile uint32_t peekAmountRev;

	mtflexrego ( 	NRND,
					&peekAmountFwd,
					&peekAmountRev,
					seeds,
			 		hw_rnd);

	std::cout << "---------------------------------------------"<< std::endl;
	std::cout << "mersenne_twister has generated random_numbers" << std::endl;
	std::cout << "---------------------------------------------"<< std::endl;
	std::cout << "comparing SW vs HW ..." << std::endl;

	// compare with software model
	printf("hw_rnd.size()    = %d\n",hw_rnd.size());
	printf("sw_rnd.size()    = %d\n",sw_rnd.size());
	if(hw_rnd.size() != sw_rnd.size())
	{
		printf("ERROR: uniform rnd sw vs hw MISMATCH\n");
		return 1;
	}

	// -------------------------------------------------------------
	int correctFwd = 0;

	for (int i = 0; i < NRND; ++i) 
	{
		uint32_t sw = sw_rnd.read();
		uint32_t hw = hw_rnd.read();
		
		if(sw != hw)
			printf("%s @[%3d]: sw = %8x vs	hw = %8x\n",(sw != hw)?"ERROR":"----",i,sw,hw);
		if(sw == hw) correctFwd++;

		if(i >= NRND-3)
			printf("%s @[%3d]: sw = %8x vs	hw = %8x\n",(sw != hw)?"ERROR":"----",i,sw,hw);
	}
    
  	printf("Computed forward random numbers : '%d/%d' correct values!\n", correctFwd, NRND);

	// -------------------------------------------------------------
  	int correctRev = 0;

  	for (int i = NRND-1; i >= 0; --i)
	{
		uint32_t sw = sw_rnd.read();
		uint32_t hw = hw_rnd.read();

		if(sw != hw)
			printf("%s @[%3d]: reverse - sw = %8x vs	hw = %8x\n",(sw != hw)?"ERROR":"----",i,sw,hw);
		if(sw == hw) correctRev++;

		if(i >= NRND-3)
			printf("%s @[%3d]: sw = %8x vs	hw = %8x\n",(sw != hw)?"ERROR":"----",i,sw,hw);
	}
  	printf("Computed reverse random numbers : '%d/%d' correct values!\n", correctRev, NRND);


	if(correctFwd == NRND && correctRev == NRND)
	{
		printf("Test passed!\n");
		return EXIT_SUCCESS;
	}
	else
	{
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	std::cout << "test passed." << std::endl;

	return 0;
}

