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

#include <stdio.h>
#include <stdint.h>
#include <hls_math.h>

#define STEPSMC 10
#define PATHSMC 2000

#define ACCUM_ELEM 10


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
	printf("lsupdate1SW\n");
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

// -----------------------------------------------------------
void lsupdate2SW	( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float discount,
						hls::stream<float> &contin_in,
						hls::stream<float> &payoff_in,
						hls::stream<float> &cashFlow_in,
						hls::stream<float> &cashFlow_out,
						hls::stream<float> &cashFlowDisc_out,
						hls::stream<float> &toAccum_out )
{
	printf("lsupdate2SW\n");

	zerosLoop:for(uint32_t path=0; path<pathsMC; ++path)
	{
#pragma HLS PIPELINE II=1 enable_flush
		// ---------------------------------
		// write to outputs
		cashFlow_out.write((float) 0.0f);

		cashFlowDisc_out.write((float) 0.0f);
	}


	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
	#pragma HLS PIPELINE II=1 enable_flush

			float continuation = contin_in.read();
			float payoff       = payoff_in.read();

			float cashFlow = cashFlow_in.read();

			float discountedCashFlow = discount * cashFlow;

			// ---------------------------------
			float newY;

			if( (payoff > (float) 0.0f) && (payoff >= (float) continuation) )
				newY = payoff;
			else
				newY = discountedCashFlow;

			// ---------------------------------
			// write to outputs
			if(step < stepsMC)
				cashFlow_out.write(newY);

			if(step < stepsMC)
				cashFlowDisc_out.write(discount * newY);

			if(step == stepsMC)
				toAccum_out.write(newY);
		}
	}

	return;
}

// -----------------------------------------------------------
void lsupdate2SWvector	( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float discount,
						hls::stream<float> &contin_in,
						hls::stream<float> &payoff_in,
						hls::stream<float> &cashFlowDisc_out,
						hls::stream<float> &toAccum_out )
{
	printf("lsupdate2SWvector\n");

	float cashFlowVector[CASHFLOW_SIZE];
#pragma HLS RESOURCE variable=cashFlowVector core=RAM_2P_BRAM

	// ----------------------------------------------------
	zerosLoop:for(uint32_t path=0; path<pathsMC; ++path)
	{
#pragma HLS PIPELINE II=1 enable_flush

		cashFlowVector[path] = (float) 0.0f;
	}

	// ----------------------------------------------------

	// ----------------------------------------------------
	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
	#pragma HLS PIPELINE II=1 enable_flush

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

		}
	}

	return;
}

// -----------------------------------------------------------
void lsupdate2SW_step	( 	const uint32_t step,
							const uint32_t stepsMC,
							const uint32_t pathsMC,
							const float discount,
							hls::stream<float> &contin_in,
							hls::stream<float> &payoff_in,
							hls::stream<float> &cashFlow_in,
							hls::stream<float> &cashFlow_out,
							hls::stream<float> &cashFlowDisc_out,
							hls::stream<float> &toAccum_out )
{
	printf("lsupdate2SW_step\n");
/**
	zerosLoop:for(uint32_t path=0; path<pathsMC; ++path)
	{
#pragma HLS PIPELINE II=1 enable_flush
		// ---------------------------------
		// write to outputs
		cashFlow_out.write((float) 0.0f);

		cashFlowDisc_out.write((float) 0.0f);
	}
*/

	//stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{

			float continuation = contin_in.read();
			float payoff       = payoff_in.read();

			float cashFlow = cashFlow_in.read();

			float discountedCashFlow = discount * cashFlow;

			// ---------------------------------
			float newY;

			if( (payoff > (float) 0.0f) && (payoff >= (float) continuation) )
				newY = payoff;
			else
				newY = discountedCashFlow;

			// ---------------------------------
			// write to outputs
			if(step < stepsMC)
				cashFlow_out.write(newY);

			if(step < stepsMC)
				cashFlowDisc_out.write(discount * newY);

			if(step == stepsMC)
				toAccum_out.write(newY);
		}
	}

	return;
}
// -----------------------------------------------------------
void accumSW(	const uint32_t stepsMC,
				const uint32_t pathsMC,
				hls::stream<float> &inData,
				hls::stream<float> &outAccum )
{
	printf("accumSW\n");

	float sums[ACCUM_ELEM];
	#pragma HLS RESOURCE variable=sums core=RAM_2P_BRAM
	#pragma HLS DEPENDENCE variable=sums false

	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
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


// -------------------------------------------------------

int main ()
{
	int stockTestRange = 40;
	float K = (float) 100.0f;
	uint32_t callPut = (uint32_t) 1;

	float T = (float) 1.0f;
	float r = (float) 0.06f;
	float dt = T / (float) STEPSMC;
	float discount = expf(r*dt);


	hls::stream<float> cashFlow_FIFO_sw;
#pragma HLS RESOURCE variable=cashFlow_FIFO_sw core=FIFO
#pragma HLS STREAM variable=cashFlow_FIFO_sw depth=8192

	hls::stream<float> cashFlowDisc_out_sw;
	hls::stream<float> toAccum_out_sw;


	hls::stream<float> cashFlow_FIFO_hw;
#pragma HLS RESOURCE variable=cashFlow_FIFO_hw core=FIFO
#pragma HLS STREAM variable=cashFlow_FIFO_hw depth=8192
	hls::stream<float> cashFlowDisc_out_hw;
	hls::stream<float> toAccum_out_hw;

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

	hls::stream<float> optionSum_sw;
	hls::stream<float> optionSum_hw;

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

	lsupdate1SW (	STEPSMC,
					PATHSMC,
					K,
					callPut,
					stock_hw,
					b0_hw,
					b1_hw,
					b2_hw,
					contin_hw,
					payoff_hw );


	volatile uint32_t peekStep;
	volatile uint32_t peekPath;

	lsupdate2regio ( 	STEPSMC,
						PATHSMC,
						discount,
						&peekStep,
						&peekPath,
						contin_hw,
						payoff_hw,
						//cashFlow_FIFO_hw,
						//cashFlow_FIFO_hw,
						cashFlowDisc_out_hw,
						toAccum_out_hw );


	accumSW( 0, PATHSMC, toAccum_out_hw, optionSum_hw  );

	// -------------------------------------------------------------
	// SW
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("SW\n");
	printf("\n");

	lsupdate1SW (	STEPSMC,
					PATHSMC,
					K,
					callPut,
					stock_sw,
					b0_sw,
					b1_sw,
					b2_sw,
					contin_sw,
					payoff_sw );


	lsupdate2SWvector ( 	STEPSMC,
					PATHSMC,
					discount,
					contin_sw,
					payoff_sw,
					cashFlowDisc_out_sw,
					toAccum_out_sw );


	accumSW( 0, PATHSMC, toAccum_out_sw, optionSum_sw  );

	// -------------------------------------------------------------
	// CHECK
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("CHECK\n");
	printf("\n");

	int errorCount = 0;

	//printf("stream_b0_hw.size() = %d\n",stream_b0_hw.size());

	printf("\n");
	printf("checking cashFlowDisc\n");
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_sw = cashFlowDisc_out_sw.read();
			float value_hw = cashFlowDisc_out_hw.read();

			if( value_sw != value_hw )
				errorCount ++;

			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}

	float optionSumValue_hw = optionSum_hw.read();
	float optionSumValue_sw = optionSum_sw.read();

	if(optionSumValue_hw != optionSumValue_sw)
		errorCount++;

	printf("optionSumValue_sw = %.4f  vs  optionSumValue_hw = %.4f\n",optionSumValue_sw,optionSumValue_hw);

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



