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

void cflowaccumregio(	const uint32_t pathsMC,
						float *totalSum,
						hls::stream<float> &inData )

{
#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=totalSum bundle=CONTROL

#pragma HLS INTERFACE axis register port=inData


	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//static float sums[N];
	float sums[NELEMENTS];
	#pragma HLS RESOURCE variable=sums core=RAM_2P_BRAM
	#pragma HLS dependence variable=sums false

	resetLoop:for(uint8_t i=0; i<NELEMENTS; ++i)
	{
#pragma HLS PIPELINE II=1

		sums[i] = (float) 0.0f;
	}

	uint8_t index = (uint8_t) 0;

	pathsLoop:for(uint32_t i=0; i<pathsMC; ++i)
	{
#pragma HLS PIPELINE II=1

		float data = inData.read();
		float oldSum = sums[index];
		float newSum = oldSum + data;

		sums[index] = newSum;

		index = (index<(NELEMENTS-1))?++index:(uint8_t)0;
	}


	float total = (float) 0.0f;

	totalLoop:for(uint8_t i=0; i<NELEMENTS;++i)
	{
#pragma HLS PIPELINE II=1
		total += sums[i];
	}

	*totalSum = total;

	return;
}

