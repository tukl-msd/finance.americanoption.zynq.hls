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

#include "correlator_hls.hpp"

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>
#include <math.h>
#include <iostream>

#define AMOUNT 10

int main() {
	float rho = 0.25;
	float sqrt_one_minus_rho2 = sqrt(1-rho*rho);

	hls::stream<float> in1;
	hls::stream<float> in2;
	hls::stream<float> out1;
	hls::stream<float> out2;

	float vector_in1[10];
	float vector_in2[10];

	std::cout << "---------------------------------------" << std::endl;
	std::cout << "generating stimuli ..." << std::endl;
	for (int i = 0; i < AMOUNT; ++i) {
		vector_in1[i] = rand();//%10;
		vector_in2[i] = rand();//%10;
		in1.write(vector_in1[i]);
		in2.write(vector_in2[i]);
		std::cout << "in1[" << i << "] = " << vector_in1[i] << " -- in2[" << i << "] = " << vector_in2[i] << std::endl;
	}
	std::cout << "---------------------------------------" << std::endl;
	std::cout << "running correlator ..." << std::endl;
	
	volatile uint32_t peekAmount;

	corramountregio(	AMOUNT,
						rho,
						sqrt_one_minus_rho2,
						&peekAmount,
						in1,
						in2,
						out1,
						out2 );

	std::cout << "---------------------------------------" << std::endl;
	std::cout << "checking values ..." << std::endl;
	if (out1.size() != AMOUNT or out2.size() != AMOUNT)
	{
		std::cout << "ERROR: wrong size" << std::endl;
	}

	int errorCount = 0;

	for (int i = 0; i < AMOUNT; ++i)
	{
		float rndin1 = vector_in1[i];
		float rndin2 = vector_in2[i];

		float hw1 = out1.read();
		float hw2 = out2.read();
		float y1 = rho * rndin1 + sqrt_one_minus_rho2 * rndin2;

		if ( (hw1 != rndin1) || (hw2 != y1) )
			errorCount++;

		printf("@i = %3d: in1 = %.3f; in2 = %.3f >> sw1 = %.3f; sw2 = %.3f >> hw1 = %.3f; hw2 = %.3f\n",i,rndin1, rndin2, rndin1,y1,hw1,hw2);
	}

	if(errorCount == 0)
	{
		printf("Testbench OK \n");
		return 0;
	}
	else
	{
		printf("Testbench ERROR \n");
		return 0;
	}

}
