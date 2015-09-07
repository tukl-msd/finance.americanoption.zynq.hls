/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier Alejandro Varela (varela@eit.uni-kl.de)
//
// Last review: 29/06/2015
//
// Using: Xilinx Vivado HLS 2014.3
//
///////////////////////////////////////////////////  


#include "boxmuller_hls.hpp"

#include <iostream>
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <hls_stream.h>
#include <hls_math.h>

#include "mt19937ar.h"

#define AMOUNT 100

int main(int argc, char *argv[]) {

	hls::stream<uint32_t> uniform_hw;
	hls::stream<uint32_t> uniform_sw2;
	hls::stream<uint32_t> uniform_sw;

	uint32_t uniformVector[AMOUNT];
	float    normalVector [AMOUNT];

	hls::stream<float> normal_hw;
	hls::stream<float> normal_sw;

	// generate seeds
	uint32_t seeds[624];
	init_genrand(0);
	generate_numbers();
	get_mt_state(seeds);

	// generate forward sequence
	for (int i = 0; i < AMOUNT; ++i)
	{
		uint32_t rn = genrand_int32();
		uniformVector[i] = rn;

		uniform_hw.write(rn);
		uniform_sw2.write(rn);
		uniform_sw.write(rn);
	}

	// complete reverse sequence only for hw version and sw2
	for (int i = AMOUNT-1; i>=0; --i)
	{
		uniform_hw.write ( uniformVector[i] );
		uniform_sw2.write( uniformVector[i] );
	}

	// --------------------------------------------------------
	// run boxmuller SW -- Forward only
	int index = 0;
	for (int i = 0; i < AMOUNT/2; ++i)
	{
		float in1 = ((float) (uniform_sw.read())) / (float) UINT2FLOAT;
		float in2 = ((float) (uniform_sw.read())) / (float) UINT2FLOAT;

		float u1 = in1;  //[0,1]-real-interval
		float u2 = in2 * (float) PIx2;  //[0,1]-real-interval

		// -------------------------------------------------
		float logf_u1 = logf(u1);
		float logf_u1_x2 = logf_u1 * ((float) 2.0f);
		float sqrtu1 = sqrtf(-logf_u1_x2);
		float arg = u2;
		// -------------------------------------------------
		float cosf_arg = cosf(arg);
		float z0 = sqrtu1 * cosf_arg;
		normal_sw.write(z0);
		// -------------------------------------------------
		float sinf_arg = sinf(arg);
		float z1 = sqrtu1 * sinf_arg;
		normal_sw.write(z1);
		// -------------------------------------------------
		normalVector[index] = z0;
		index++;
		normalVector[index] = z1;
		index++;
	}

	// complete Reverse sequence for normal_sw
	for (int i = AMOUNT-1; i>=0; --i)
	{
		normal_sw.write ( normalVector[i] );
	}


	// ------------------------------------------------------------------
	// run boxmuller HW
	volatile uint32_t peekAmount;

	bmflexregio (	AMOUNT,
					AMOUNT/2,
					&peekAmount,
					uniform_hw,
					normal_hw);

	// ------------------------------------------------------------------
	// check
	int errorCount = 0;

	printf("CHECKING BoxMuller ...\n");
	printf("-------------- Fwd  start  ------------------\n");
	for (int i = 0, j = (AMOUNT*2-1); i < AMOUNT*2; ++i,--j)
	{
		uint32_t unifValue = uniform_sw2.read();
		float n_sw = normal_sw.read();
		float n_hw = normal_hw.read();

		// -----------------------------------
		int t = (i < AMOUNT )?i:j;
		printf("%s @[%4d]: uint32_t = 0x%8x >> float_sw = %.2f vs float_hw = %.2f  -- diff = %e\n",(n_sw != n_hw)?"ERROR":"----",t,unifValue,n_sw,n_hw,n_hw-n_sw);
		// -----------------------------------

		if(n_sw != n_hw)
			if(fabsf(n_sw-n_hw) > (float) 5.0e-6)
				errorCount++;

		if(i == AMOUNT-1)
		{
			printf("-------------- Fwd complete -----------------\n");
			printf("-------------- Rev  start  ------------------\n");
		}

		if(i == ((AMOUNT*2)-1))
			printf("-------------- Rev complete -----------------\n");

	}
	
	// ------------------------------------------------------------------
	if(errorCount == 0)
	{
		printf("Test passed!\n");
		return EXIT_SUCCESS;
	}
	else
	{
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	return 0;
}

