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

#include "cholesky_hls.hpp"

#include <stdio.h>
#include <stdint.h>
#include <hls_math.h>

#define STEPSMC 10
#define PATHSMC 100

#define ACCUM_ELEM 10

// -----------------------------------------------------------
void accumSW(	const uint32_t stepsMC,
				const uint32_t pathsMC,
				hls::stream<float> &inData,
				hls::stream<float> &outAccum )
{
	float sums[ACCUM_ELEM];
	#pragma HLS RESOURCE variable=sums core=RAM_2P_BRAM
	#pragma HLS DEPENDENCE variable=sums false

	stepsLoop:for(uint32_t step=0; step<stepsMC; ++step)
	{
		// ------------------------------------------
		resetLoop:for(uint8_t i=0; i<ACCUM_ELEM; ++i)
		{
	#pragma HLS PIPELINE II=1 enable_flush

			sums[i] = (float) 0.0f;
		}

		// ------------------------------------------
		uint8_t index = (uint8_t) 0;

		pathsLoop:for(uint32_t i=0; i<pathsMC; ++i)
		{
	#pragma HLS PIPELINE II=1 enable_flush

			float data = inData.read();
			float oldSum = sums[index];
			float newSum = oldSum + data;

			sums[index] = newSum;

			index = (index<(ACCUM_ELEM-1))?++index:(uint8_t)0;
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
	}

	return;
}

// -----------------------------------------------------------


void lsdatagenSW ( 	const uint32_t stepsMC,
					const uint32_t pathsMC,
					const float K,
					const uint32_t callPut,
					hls::stream<float> &stock,
					hls::stream<float> &cashFlow,
					hls::stream<float> &stream_x0,
					hls::stream<float> &stream_x1,
					hls::stream<float> &stream_x2,
					hls::stream<float> &stream_x3,
					hls::stream<float> &stream_x4,
					hls::stream<float> &stream_y,
					hls::stream<float> &stream_yx,
					hls::stream<float> &stream_yx2 )
{

	stepsLoop:for(uint32_t step=0; step<stepsMC; ++step)
	{
		pathsLoop:for(uint32_t i=0; i<pathsMC; ++i)
		{
	#pragma HLS PIPELINE II=1 enable_flush

			float s = stock.read();
			float y = cashFlow.read();

			// ---------------------------------
			// in-the-money calculation
			float diff = (s-K);

			float payoff;

			if(callPut == 0)
				payoff = diff;
			else
				payoff = -diff;

			bool inTheMoney;

			if( payoff > 0.0f )
				inTheMoney = true;
			else
				inTheMoney = false;

			// ---------------------------------
			// basis functions
			float s2 = s*s;

			float x0;
			float x1;
			float x2;

			if(inTheMoney == true)
			{
				x0 = (float) 1.0f;
				x1 = (float) s;
				x2 = (float) s2;
			}
			else
			{
				x0 = (float) 0.0f;
				x1 = (float) 0.0f;
				x2 = (float) 0.0f;
			}

			// remaining multipliers
			float x3  = x1*x2;
			float x4  = x2*x2;
			float yx  =  y*x1;
			float yx2 =  y*x2;

			// write to streams
			stream_x0.write(x0);
			stream_x1.write(x1);
			stream_x2.write(x2);
			stream_x3.write(x3);
			stream_x4.write(x4);
			stream_y.write(y);
			stream_yx.write(yx);
			stream_yx2.write(yx2);
		}
	}
	return;
}
// ---------------------------------------------------------

void choleskySW   ( const uint32_t stepsMC,
					hls::stream<float> &in_one,
					hls::stream<float> &in_x,
					hls::stream<float> &in_x2,
					hls::stream<float> &in_x3,
					hls::stream<float> &in_x4,
					hls::stream<float> &in_y,
					hls::stream<float> &in_yx,
					hls::stream<float> &in_yx2,
					hls::stream<float> &out_b0,  // output
					hls::stream<float> &out_b1,	// output
					hls::stream<float> &out_b2 ) // output
{

	for(uint32_t step=1; step<=stepsMC; ++step)
	{
//#pragma HLS PIPELINE II=1 enable_flush

		float one  = in_one.read();
		float x    = in_x.read();
		float x2   = in_x2.read();
		float x3   = in_x3.read();
		float x4   = in_x4.read();
		float y    = in_y.read();
		float yx   = in_yx.read();
		float yx2  = in_yx2.read();

		// Cholesky 3x3
		//% one
		float l11 = sqrtf(one);
//#pragma HLS RESOURCE variable=l11 core=FSqrt

		//%two
		float l21 = x/l11;
		float l31 = x2/l11;
		//%three
		float temp1 = l21*l21;
//#pragma HLS RESOURCE variable=temp1 core=FMul_nodsp
		float temp2 = x2-temp1;
//#pragma HLS RESOURCE variable=temp2 core=FAddSub_fulldsp
		float l22 = sqrtf(temp2);
//#pragma HLS RESOURCE variable=l22 core=FSqrt

		float temp3 = l21*l31;
//#pragma HLS RESOURCE variable=temp3 core=FMul_nodsp
		float temp4 = x3-temp3;
//#pragma HLS RESOURCE variable=temp4 core=FAddSub_fulldsp
		float l32 = (temp4)/l22;
		//float l22 = sqrtf(x2-l21*l21);
		//float l32 = (x3-l21*l31)/l22;
		//%four
		float l33 = sqrtf((x4-l31*l31)-l32*l32);

		//% Forward substitution
		float z1  = y /l11;
		float z2  = (yx-l21*z1)/l22;
		float z3  = (yx2-l31*z1-l32*z2)/l33;
		//% Backward substitution
		float b2  = z3 / l33;
		float b1  = (z2 - l32*b2) / l22;
		float b0  = (z1 - l21*b1 - l31*b2)/l11;

		out_b0.write(b0);
		out_b1.write(b1);
		out_b2.write(b2);

	}
	return;
}

// -------------------------------------------------------


int main ()
{
	int stockTestRange = 40;
	float K = (float) 100.0f;
	uint32_t callPut = (uint32_t) 1;
	/*
	float one  = 9765.0f;
	float x1 = 8.7905e+05f; 
	float x2 = 7.9318e+07f;
	float x3 = 7.1736e+09f;
	float x4 = 6.5028e+11f;
	
	float y   = 9.6989e+04f;
	float yx = 8.5469e+06f;
	float yx2 = 7.5476e+08f;
	*/
	volatile float b0_vector[CHOLESKY_VECTOR_SIZE];
	volatile float b1_vector[CHOLESKY_VECTOR_SIZE];
	volatile float b2_vector[CHOLESKY_VECTOR_SIZE];
	volatile uint32_t peekStep;

	hls::stream<float> stock_sw;
	hls::stream<float> stock_hw;
	hls::stream<float> cashFlow_sw;
	hls::stream<float> cashFlow_hw;

	hls::stream<float> stream_x0_sw;
	hls::stream<float> stream_x1_sw;
	hls::stream<float> stream_x2_sw;
	hls::stream<float> stream_x3_sw;
	hls::stream<float> stream_x4_sw;
	hls::stream<float> stream_y_sw;
	hls::stream<float> stream_yx_sw;
	hls::stream<float> stream_yx2_sw;

	hls::stream<float> stream_x0_hw;
	hls::stream<float> stream_x1_hw;
	hls::stream<float> stream_x2_hw;
	hls::stream<float> stream_x3_hw;
	hls::stream<float> stream_x4_hw;
	hls::stream<float> stream_y_hw;
	hls::stream<float> stream_yx_hw;
	hls::stream<float> stream_yx2_hw;

	hls::stream<float> accum_x0_sw;
	hls::stream<float> accum_x1_sw;
	hls::stream<float> accum_x2_sw;
	hls::stream<float> accum_x3_sw;
	hls::stream<float> accum_x4_sw;
	hls::stream<float> accum_y_sw;
	hls::stream<float> accum_yx_sw;
	hls::stream<float> accum_yx2_sw;

	hls::stream<float> accum_x0_hw;
	hls::stream<float> accum_x1_hw;
	hls::stream<float> accum_x2_hw;
	hls::stream<float> accum_x3_hw;
	hls::stream<float> accum_x4_hw;
	hls::stream<float> accum_y_hw;
	hls::stream<float> accum_yx_hw;
	hls::stream<float> accum_yx2_hw;

	hls::stream<float> out_b0_sw;
	hls::stream<float> out_b1_sw;
	hls::stream<float> out_b2_sw;

	hls::stream<float> out_b0_hw;
	hls::stream<float> out_b1_hw;
	hls::stream<float> out_b2_hw;

	// GENERATE INPUT STIMULI
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{

			float tempStock    = (float) ((float) (rand()%(stockTestRange*100)))/100.0f + (K-stockTestRange/2);
			float tempCashFlow = (float) ((float) (rand()%(stockTestRange*100)))/100.0f - (  stockTestRange/2);
			tempCashFlow = fmaxf(tempCashFlow,0.0f);

			stock_sw.write(tempStock);
			stock_hw.write(tempStock);

			cashFlow_sw.write(tempCashFlow);
			cashFlow_hw.write(tempCashFlow);
		}
	}

	lsdatagenSW ( 	STEPSMC,
					PATHSMC,
					K,
					callPut,
					stock_hw,
					cashFlow_hw,
					stream_x0_hw,
					stream_x1_hw,
					stream_x2_hw,
					stream_x3_hw,
					stream_x4_hw,
					stream_y_hw,
					stream_yx_hw,
					stream_yx2_hw );
	accumSW( STEPSMC, PATHSMC, stream_x0_hw,  accum_x0_hw  );
	accumSW( STEPSMC, PATHSMC, stream_x1_hw,  accum_x1_hw  );
	accumSW( STEPSMC, PATHSMC, stream_x2_hw,  accum_x2_hw  );
	accumSW( STEPSMC, PATHSMC, stream_x3_hw,  accum_x3_hw  );
	accumSW( STEPSMC, PATHSMC, stream_x4_hw,  accum_x4_hw  );
	accumSW( STEPSMC, PATHSMC, stream_y_hw,   accum_y_hw   );
	accumSW( STEPSMC, PATHSMC, stream_yx_hw,  accum_yx_hw  );
	accumSW( STEPSMC, PATHSMC, stream_yx2_hw, accum_yx2_hw );


	// -------------------------------------------------------------
	// HW
	// -------------------------------------------------------------
	choleskyregio ( STEPSMC,
					b0_vector,
					b1_vector,
					b2_vector,
					&peekStep,
					accum_x0_hw,
					accum_x1_hw,
					accum_x2_hw,
					accum_x3_hw,
					accum_x4_hw,
					accum_y_hw,
					accum_yx_hw,
					accum_yx2_hw,
					out_b0_hw,
					out_b1_hw,
					out_b2_hw );
	// -------------------------------------------------------------
	// SW
	// -------------------------------------------------------------
	lsdatagenSW ( 	STEPSMC,
						PATHSMC,
						K,
						callPut,
						stock_sw,
						cashFlow_sw,
						stream_x0_sw,
						stream_x1_sw,
						stream_x2_sw,
						stream_x3_sw,
						stream_x4_sw,
						stream_y_sw,
						stream_yx_sw,
						stream_yx2_sw );
		accumSW( STEPSMC, PATHSMC, stream_x0_sw,  accum_x0_sw  );
		accumSW( STEPSMC, PATHSMC, stream_x1_sw,  accum_x1_sw  );
		accumSW( STEPSMC, PATHSMC, stream_x2_sw,  accum_x2_sw  );
		accumSW( STEPSMC, PATHSMC, stream_x3_sw,  accum_x3_sw  );
		accumSW( STEPSMC, PATHSMC, stream_x4_sw,  accum_x4_sw  );
		accumSW( STEPSMC, PATHSMC, stream_y_sw,   accum_y_sw   );
		accumSW( STEPSMC, PATHSMC, stream_yx_sw,  accum_yx_sw  );
		accumSW( STEPSMC, PATHSMC, stream_yx2_sw, accum_yx2_sw );

	choleskySW   ( STEPSMC,
					accum_x0_sw,
					accum_x1_sw,
					accum_x2_sw,
					accum_x3_sw,
					accum_x4_sw,
					accum_y_sw,
					accum_yx_sw,
					accum_yx2_sw,
					out_b0_sw,
					out_b1_sw,
					out_b2_sw );


	// -------------------------------------------------------------
	// CHECK
	// -------------------------------------------------------------
	int countError = 0;

	printf("out_b0_sw.size() = %d\n",out_b0_sw.size());
	printf("out_b1_sw.size() = %d\n",out_b1_sw.size());
	printf("out_b2_sw.size() = %d\n",out_b2_sw.size());
	printf("out_b0_hw.size() = %d\n",out_b0_hw.size());
	printf("out_b1_hw.size() = %d\n",out_b1_hw.size());
	printf("out_b2_hw.size() = %d\n",out_b2_hw.size());

	for (int step=0; step < STEPSMC; ++step)
	{
		float b0_sw = out_b0_sw.read();
		float b1_sw = out_b1_sw.read();
		float b2_sw = out_b2_sw.read();

		float b0_hw = out_b0_hw.read();
		float b1_hw = out_b1_hw.read();
		float b2_hw = out_b2_hw.read();

		if( (b0_hw != b0_sw) || (b1_hw != b1_sw) || (b2_hw != b2_sw) )
		{
			countError++;
		}
		printf("-------------------------------------\n");
		printf("@ step = %d\n",step);
		printf("--b0_hw = %.6f  vs  b0_sw = %.6f\n", b0_hw, b0_sw);
		printf("--b1_hw = %.6f  vs  b1_sw = %.6f\n", b1_hw, b1_sw);
		printf("--b2_hw = %.6f  vs  b2_sw = %.6f\n", b2_hw, b2_sw);

	}

	if(countError == 0)
	{
		printf("Testbench passed !!\n");
		return 0;
	}
	else
	{
		printf("Testbench ERROR !!\n");
		return 1;
	}

}


