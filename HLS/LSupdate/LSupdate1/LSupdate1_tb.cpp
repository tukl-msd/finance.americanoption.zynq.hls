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

#include <stdio.h>
#include <stdint.h>
#include <hls_math.h>

#define STEPSMC 5
#define PATHSMC 100

#define ACCUM_ELEM 10
#define CHOLESKY_VECTOR_SIZE 1024



// -------------------------------------------------------

void lsupdate1SW  ( const uint32_t stepsMC,
					const uint32_t pathsMC,
					const float K,
					const uint32_t callPut,
					hls::stream<float> &stock,
					hls::stream<float> &b0_in,
					hls::stream<float> &b1_in,
					hls::stream<float> &b2_in,
					hls::stream<float> &contin_out,
					hls::stream<float> &payoff_out )
{
	printf("lsupdateSW\n");
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
	#pragma HLS PIPELINE II=1 enable_flush

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

		}
	}

	return;
}
// -------------------------------------------------------

int main ()
{
	int stockTestRange = 40;
	float K = (float) 100.0f;
	uint32_t callPut = (uint32_t) 1;

	hls::stream<float> stock_sw;
	hls::stream<float> stock_hw;

	hls::stream<float> contin_sw;
	hls::stream<float> contin_hw;

	hls::stream<float> payoff_sw;
	hls::stream<float> payoff_hw;

	hls::stream<float> b0_sw;
	hls::stream<float> b1_sw;
	hls::stream<float> b2_sw;

	hls::stream<float> b0_hw;
	hls::stream<float> b1_hw;
	hls::stream<float> b2_hw;


	// GENERATE INPUT STIMULI
	printf("======================================================\n");
	printf("Generating Stimuli\n");
	for(int step=0; step<=STEPSMC; ++step)
	{
		if(step > 0)
		{
			b0_sw.write(159.2057f);
			b1_sw.write(-2.3288f);
			b2_sw.write(0.0074f);

			b0_hw.write(159.2057f);
			b1_hw.write(-2.3288f);
			b2_hw.write(0.0074f);
		}

		for(int path=0; path<PATHSMC; ++path)
		{
			float tempStock    = (float) ((float) (rand()%(stockTestRange*100)))/100.0f + (K-stockTestRange/2);

			stock_sw.write(tempStock);
			stock_hw.write(tempStock);
		}
	}

	// -------------------------------------------------------------
	// HW
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("HW\n");
	printf("\n");
	volatile uint32_t peekStep;
	volatile uint32_t peekPath;

	lsupdate1regio (STEPSMC,
					PATHSMC,
					K,
					callPut,
					&peekStep,
					&peekPath,
					stock_hw,
					b0_hw,
					b1_hw,
					b2_hw,
					contin_hw,
					payoff_hw );

	// -------------------------------------------------------------
	// SW
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("SW\n");
	printf("\n");

	lsupdate1SW (STEPSMC,
				PATHSMC,
				K,
				callPut,
				stock_sw,
				b0_sw,
				b1_sw,
				b2_sw,
				contin_sw,
				payoff_sw );


	// -------------------------------------------------------------
	// CHECK
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("CHECK\n");
	printf("\n");

	int errorCount = 0;

	for(int step=0; step<=STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float contValue_sw = contin_sw.read();
			float contValue_hw = contin_hw.read();

			if( contValue_sw != contValue_hw )
				errorCount ++;

			printf("%s:@step = %3d, path = %3d >> contValue_sw = %.3f  vs  contValue_hw = %.3f\n",(contValue_sw==contValue_hw)?"-----":"ERROR",step,path,contValue_sw,contValue_hw);
		}
	}

	for(int step=0; step<=STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float payoffValue_sw = payoff_sw.read();
			float payoffValue_hw = payoff_hw.read();

			if( payoffValue_sw != payoffValue_hw )
				errorCount ++;

			printf("%s:@step = %3d, path = %3d >> payoffValue_sw = %.3f  vs  payoffValue_hw = %.3f\n",(payoffValue_sw==payoffValue_hw)?"-----":"ERROR",step,path,payoffValue_sw,payoffValue_hw);
		}
	}

	if(errorCount == 0)
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


