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

#include "accumulator_hls.hpp"

#include <hls_stream.h>
#include <stdint.h>
#include <string.h>

void accumregio(	const uint32_t stepsMC,
					const uint32_t pathsMC,
					volatile uint32_t *peekStep,
					volatile uint32_t *peekPath,
					hls::stream<float> &inData,
					hls::stream<float> &outAccum )

{

//#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
//#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
//#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
//#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
//#pragma HLS INTERFACE s_axilite port=peekPath bundle=CONTROL

//#pragma HLS INTERFACE axis register port=inData
//#pragma HLS INTERFACE axis register port=outAccum


	float sums[ACCUM_ELEM];
	#pragma HLS RESOURCE variable=sums core=RAM_2P_BRAM
	#pragma HLS DEPENDENCE variable=sums false

	*peekStep = 0xFFFFFFFF;
	*peekPath = 0xFFFFFFFF;

	stepsLoop:for(uint32_t step=0; step<stepsMC; ++step)
	{
//#pragma HLS PIPELINE enable_flush

		resetLoop:for(uint8_t i=0; i<ACCUM_ELEM; ++i)
		{
	#pragma HLS PIPELINE II=1

			sums[i] = (float) 0.0f;
		}

		*peekStep = step;

		// ------------------------------------------
		uint8_t index = (uint8_t) 0;

		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
	#pragma HLS PIPELINE II=1

			float data = inData.read();
			float oldSum = sums[index];
			float newSum = oldSum + data;

			sums[index] = newSum;

			index = (index<(ACCUM_ELEM-1))?++index:(uint8_t)0;

			*peekPath = path;
		}

		// ------------------------------------------
		float totalSum = (float) 0.0f;

		totalLoop:for(uint8_t i=0; i<ACCUM_ELEM;++i)
		{
	#pragma HLS PIPELINE II=1
			totalSum += sums[i];
		}

		// ------------------------------------------
		outAccum.write(totalSum);

		//*peekStep = step;
	}

	return;
}

