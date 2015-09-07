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

#include "LSupdate1_hls.hpp"

#include <hls_math.h>

void lsupdate1regio ( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float K,
						const uint32_t callPut,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &stock,
						hls::stream<float> &b0_in,
						hls::stream<float> &b1_in,
						hls::stream<float> &b2_in,
						hls::stream<float> &contin_out,
						hls::stream<float> &payoff_out )
{
#pragma HLS INTERFACE axis register port=stock
#pragma HLS INTERFACE axis register port=b0_in
#pragma HLS INTERFACE axis register port=b1_in
#pragma HLS INTERFACE axis register port=b2_in
#pragma HLS INTERFACE axis register port=contin_out
#pragma HLS INTERFACE axis register port=payoff_out

#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=K bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=callPut bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath bundle=CONTROL

	*peekStep = 0xFFFFFFFF;
	*peekPath = 0xFFFFFFFF;

	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		// ---------------------------------
		// continuation value
		float b0;
		float b1;
		float b2;

		if(step == 0)
		{
			b0 = (float) 0.0f;
			b1 = (float) 0.0f;
			b2 = (float) 0.0f;
		}
		else
		{
			b0 = b0_in.read();
			b1 = b1_in.read();
			b2 = b2_in.read();
		}


		// -------------------------------------

		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
	#pragma HLS PIPELINE II=1

			float s = stock.read();

			// ---------------------------------
			// payoff calculation
			float diff = (s-K);

			float callPutDiff;

			if(callPut == 0)
				callPutDiff = diff;
			else
				callPutDiff = -diff;

			float payoff = fmaxf(callPutDiff, (float) 0.0f);

			// ---------------------------------
			// basis functions
			float s2 = s*s;

			float x0 = (float) 1.0f;
			float x1 = s;
			float x2 = s2;

			// ---------------------------------
			// continuation value

			float continuation = b0*x0 + b1*x1 + b2*x2;

			// ---------------------------------
			// write to output
			contin_out.write(continuation);
			payoff_out.write(payoff);

			*peekStep = step;
			*peekPath = path;
		}
	}

	return;
}
