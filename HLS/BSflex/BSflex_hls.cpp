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


#include "BSflex_hls.hpp"

#include <hls_math.h>

#define BLOCK_SIZE 32768

void bsflexregio (	uint32_t stepsMC,
					uint32_t pathsMC,
					//uint32_t stepsMC_x2,
					float S0,
					float m1,
					float m2,
					volatile uint32_t *peekStep,
					volatile uint32_t *peekPath,
					hls::stream<float> &rn_in,
					hls::stream<float> &stock )
					//hls::stream<float> &stockDebug )
{
#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=m1 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=m2 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=S0 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
//#pragma HLS INTERFACE s_axilite port=stepsMC_x2 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath bundle=CONTROL

#pragma HLS INTERFACE axis port=stock
//#pragma HLS INTERFACE axis port=stockDebug
#pragma HLS INTERFACE axis port=rn_in


	float lastStepStock[BLOCK_SIZE];
#pragma HLS RESOURCE variable=lastStepStock core=RAM_2P_BRAM
#pragma HLS dependence variable=lastStepStock false
	

	// Forward Loop
	loopStepFwd:for (uint32_t step = 1; step <= stepsMC; ++step)
	{
		//*peekStep = step;

		loopPathFwd:for (uint32_t i = 1; i <= pathsMC; ++i)
		{
#pragma HLS PIPELINE II=1

			// previous step stock
			float prevStepStock;
			if (step == 1)
				prevStepStock = S0;
			else
				prevStepStock = lastStepStock[i];

			// exponent and exp result
			float z1 = rn_in.read();
			float temp = m1 + m2 * z1;
			float exponent = temp;

			float expResult = hls::expf(exponent);
			//#pragma HLS RESOURCE variable=expResult core=FExp_nodsp

			// newStock value
			float newStock = prevStepStock * expResult;

			//if(step == stepsMC)
			//	stock.write(newStock);

			//stockDebug.write(newStock);

			// store newStock as lastStepStock[i];
			lastStepStock[i] = newStock;

			*peekStep = step;
			*peekPath = i;
		}
	}

	*peekStep = stepsMC;

	// Output Maturity values
	loopMaturity:for (uint32_t i = pathsMC; i >0; --i)
	{
#pragma HLS PIPELINE II=1
		stock.write(lastStepStock[i]);
	}

	// Reverse Loop
	loopStepRev:for (uint32_t step = stepsMC; step > 0; --step)
	{
		//*peekStep = step;

		loopPathRev:for (uint32_t i = pathsMC; i >0; --i)
		{
#pragma HLS PIPELINE II=1

			// previous step stock
			float prevStepStock = lastStepStock[i];

			// exponent and exp result
			float z1 = rn_in.read();
			float temp = m1 + m2 * z1;
			float exponent = -temp;

			float expResult = hls::expf(exponent);
			//#pragma HLS RESOURCE variable=expResult core=FExp_nodsp

			// newStock value
			float newStock = prevStepStock * expResult;

			stock.write(newStock);

			//stockDebug.write(newStock);

			// store newStock as lastStepStock[i];
			lastStepStock[i] = newStock;

			*peekStep = step;
			*peekPath = i;
		}
	}
	return;
}

