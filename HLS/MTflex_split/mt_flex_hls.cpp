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
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define UPPER_MASK 0x80000000
#define LOWER_MASK 0x7FFFFFFF
#define MAGIC1 0x9908B0DF
#define Y_MASK 0xFFFFFFFE


void mtflexrego ( 	const uint32_t amount,
					volatile uint32_t *peekAmountFwd,
					volatile uint32_t *peekAmountRev,
					unsigned int seeds[MT_N],
		 			hls::stream<uint32_t> &uniform )
{

#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=amount bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekAmountFwd bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekAmountRev bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=seeds bundle=CONTROL
#pragma HLS INTERFACE axis register port=uniform

	unsigned int index_minus_0, index_minus_396, index_minus_397;
	bool index_minus_396_valid = false;

	uint16_t cnt396 = 0;
	uint16_t cnt396bis = 0;
	uint16_t cnt624 = 0;
	uint16_t cnt227 = 0;
	uint16_t cntStates = 0;

	uint16_t cnt227read = 0;
	uint16_t cnt227write = 0;

	unsigned int buffer396[MT_M];
	#pragma HLS dependence variable=buffer396 false

	unsigned int buffer227[MT_M];//N-M];
	#pragma HLS dependence variable=buffer227 false

	unsigned int states[MT_N];
	#pragma HLS dependence variable=states false

	unsigned int last227;

	//unsigned int result = 0;

	// -----------------------------------------------------------
	// FORWARD OPERATION
	// -----------------------------------------------------------


	for (uint32_t i = 0; i < amount; ++i)
	{
#pragma HLS PIPELINE II=1 enable_flush
		
		if (i < MT_N)
		{
			index_minus_0 = seeds[i];
		}
		else
		{
			index_minus_0 = buffer227[cnt227read];//last227;
			cnt227read = (cnt227read < (MT_M-1))? (cnt227read+1) : 0;
		}

		// tempering
		unsigned int y0 = index_minus_0; //seeds[cnt];
		unsigned int y1 = y0 ^ (y0 >> 11);
		unsigned int y2 = y1 ^ ((y1 << 7) & 0x9D2C5680);
		unsigned int y3 = y2 ^ ((y2 << 15) & 0xEFC60000);
		unsigned int y4 = y3 ^ (y3 >> 18);

		// output rnd
		uniform.write(y4);

		// update state
		if (i >= 396)  
		{
			index_minus_397 = index_minus_396;
			index_minus_396 = buffer396[cnt396bis];
			cnt396bis = (cnt396bis < (MT_M-1))? (cnt396bis+1) : 0;

			// update state
			unsigned int index_0 = index_minus_397;
			unsigned int index_1 = index_minus_396;
			unsigned int index_m = index_minus_0;

			unsigned int mixbits = (index_0 & 0x80000000) | (index_1 & 0x7FFFFFFF);
			unsigned int twist = ((index_1 & 1) ? 0x9908B0DF : 0) ^ (mixbits >> 1);
			unsigned int result = index_m ^ twist;
			//result = index_m ^ twist;

			if(index_minus_396_valid)
			{
				buffer227[cnt227write] = result;
				cnt227write = (cnt227write < (MT_M-1))? (cnt227write+1) : 0;
				
			}
			index_minus_396_valid = true;
		}

		buffer396[cnt396] = index_minus_0;
		cnt396 = (cnt396 < (MT_M-1))? (cnt396+1) : 0;

		states[cntStates] = index_minus_0;
		cntStates = (cntStates < (MT_N-1))? (cntStates+1) : 0;

		*peekAmountFwd = i;
	}

	// -----------------------------------------------------------
	// REVERSE OPERATION
	// -----------------------------------------------------------
	cnt396 = 0;
	cnt396bis = 0;
	cnt624 = 0;
	cnt227 = 0;
	cntStates = (cntStates > 0)? (cntStates-1) : (MT_N-1);

	index_minus_396_valid = false;

	cnt227read = 0;
	cnt227write = 0;

	uint32_t p = 0;

	for (uint32_t i = 0; i < amount; ++i)
	{
#pragma HLS PIPELINE II=1 enable_flush

		if (i < MT_N)
		{
			index_minus_0 = states[cntStates];//seeds[i];
		}
		else
		{
			index_minus_0 = buffer227[cnt227read];//last227;
			cnt227read = (cnt227read < (MT_M-1))? (cnt227read+1) : 0;
		}

		// tempering
		unsigned int y0 = index_minus_0; //seeds[cnt];
		unsigned int y1 = y0 ^ (y0 >> 11);
		unsigned int y2 = y1 ^ ((y1 << 7) & 0x9D2C5680);
		unsigned int y3 = y2 ^ ((y2 << 15) & 0xEFC60000);
		unsigned int y4 = y3 ^ (y3 >> 18);

		// output rnd
		uniform.write(y4);

		// update state
		if (i >= 226)//396)
		{
			// when it reaches i == 227 then index_minus_397 has the first correct value
			// for this void iteration to compute p use index_minus_397 and index_minus_0
			// but do NOT store the value
			index_minus_397 = index_minus_396;
			index_minus_396 = buffer396[cnt396bis];
			cnt396bis = (cnt396bis < (MT_M-1))? (cnt396bis+1) : 0;

			// Inverse computation of internal states
			unsigned int valueA = index_minus_397;//index_minus_0;//index_minus_397;
			unsigned int valueB = index_minus_0;//index_minus_397;//index_minus_0;

			uint32_t z = valueA ^ valueB;
			uint32_t x = z & UPPER_MASK;
			uint32_t y = z ^ ((x==UPPER_MASK)?MAGIC1:0x00000000);
			uint32_t q = ((y << 1) & Y_MASK) | ((x==UPPER_MASK)?0x00000001:0x00000000);

			uint32_t result = (p & UPPER_MASK) | (q & LOWER_MASK);
			p = q;

			if(index_minus_396_valid)
			{
				buffer227[cnt227write] = result;
				cnt227write = (cnt227write < (MT_M-1))? (cnt227write+1) : 0;
			}
			if(i >= 227)
				index_minus_396_valid = true;
		}

		buffer396[cnt396] = index_minus_0;
		cnt396 = (cnt396 < (MT_M-1))? (cnt396+1) : 0;

		states[cntStates] = index_minus_0;
		cntStates = (cntStates > 0)? (cntStates-1) : (MT_N-1);

		*peekAmountRev = i;
	}
	return;
}


