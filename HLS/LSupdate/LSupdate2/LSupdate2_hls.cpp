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

#include "LSupdate2_hls.hpp"

#include <hls_math.h>

void lsupdate2regio ( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float discount,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &contin_in,
						hls::stream<float> &payoff_in,
						hls::stream<float> &cashFlowDisc_out,
						hls::stream<float> &toAccum_out )
{

#pragma HLS INTERFACE axis register port=contin_in
#pragma HLS INTERFACE axis register port=payoff_in
#pragma HLS INTERFACE axis register port=cashFlowDisc_out
#pragma HLS INTERFACE axis register port=toAccum_out

#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=discount bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath bundle=CONTROL


	float cashFlowVector[CASHFLOW_SIZE];
#pragma HLS RESOURCE variable=cashFlowVector core=RAM_2P_BRAM
#pragma HLS DEPENDENCE variable=cashFlowVector false

	// ----------------------------------------------------
	*peekStep = 0xFFFFFFFF;

	// ----------------------------------------------------
	zerosLoop:for(uint32_t path=0; path<pathsMC; ++path)
	{
#pragma HLS PIPELINE II=1

		cashFlowVector[path] = (float) 0.0f;
	}

	// ----------------------------------------------------
	*peekStep = 0xCCCCCCCC;

	// ----------------------------------------------------
	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		*peekStep = step;

		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
	#pragma HLS PIPELINE II=1

			uint32_t indexRead = path;
			uint32_t indexWrite = path;

			float continuation = contin_in.read();
			float payoff       = payoff_in.read();

			float cashFlow = cashFlowVector[indexRead];

			float discountedCashFlow = discount * cashFlow;

			// ---------------------------------
			float newY;

			if( (payoff > (float) 0.0f) && (payoff >= (float) continuation) )
				newY = payoff;
			else
				newY = discountedCashFlow;

			// ---------------------------------
			// write to outputs
			cashFlowVector[indexWrite] = newY;

			if(step < stepsMC)
				cashFlowDisc_out.write(discount * newY);

			if(step == stepsMC)
				toAccum_out.write(newY);

			//*peekStep = step;
			*peekPath = path;
		}
	}

	return;
}
