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

#include "accumWrapper_hls.hpp"
#include "accumulator_hls.hpp"

#include <stdio.h>
//-----------------------
#include <hls_stream.h>
#include <stdlib.h>
#include <stdint.h>
//-----------------------

#define STEPSMC 10
#define PATHSMC 100


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

// -----------------------------------------------------------

//int main(int argc, char *argv[]) {
int main() {

	int stockTestRange = 40;
	float K = (float) 100.0f;
	uint32_t callPut = (uint32_t) 1;

	// ------------------------------------------------------------------------------------
	// PREPARE
	// ------------------------------------------------------------------------------------
	float sums[ACCUM_ELEM];

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

	volatile uint32_t peekStep_x0;
	volatile uint32_t peekStep_x1;
	volatile uint32_t peekStep_x2;
	volatile uint32_t peekStep_x3;
	volatile uint32_t peekStep_x4;
	volatile uint32_t peekStep_y;
	volatile uint32_t peekStep_yx;
	volatile uint32_t peekStep_yx2;
	volatile uint32_t peekPath_x0;
	volatile uint32_t peekPath_x1;
	volatile uint32_t peekPath_x2;
	volatile uint32_t peekPath_x3;
	volatile uint32_t peekPath_x4;
	volatile uint32_t peekPath_y;
	volatile uint32_t peekPath_yx;
	volatile uint32_t peekPath_yx2;
//#pragma HLS stream depth=1000 variable=inData_hw

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

	// ------------------------------------------------------------------------------------
	// HW
	// ------------------------------------------------------------------------------------

	lsrdcaccumregio(	STEPSMC,
						PATHSMC,
						&peekStep_x0,
						&peekStep_x1,
						&peekStep_x2,
						&peekStep_x3,
						&peekStep_x4,
						&peekStep_y,
						&peekStep_yx,
						&peekStep_yx2,
						&peekPath_x0,
						&peekPath_x1,
						&peekPath_x2,
						&peekPath_x3,
						&peekPath_x4,
						&peekPath_y,
						&peekPath_yx,
						&peekPath_yx2,
						stream_x0_hw,
						stream_x1_hw,
						stream_x2_hw,
						stream_x3_hw,
						stream_x4_hw,
						stream_y_hw,
						stream_yx_hw,
						stream_yx2_hw,
						accum_x0_hw,
						accum_x1_hw,
						accum_x2_hw,
						accum_x3_hw,
						accum_x4_hw,
						accum_y_hw,
						accum_yx_hw,
						accum_yx2_hw );

	// ------------------------------------------------------------------------------------
	// SW
	// ------------------------------------------------------------------------------------


	accumSW( STEPSMC, PATHSMC, stream_x0_sw,  accum_x0_sw  );
	accumSW( STEPSMC, PATHSMC, stream_x1_sw,  accum_x1_sw  );
	accumSW( STEPSMC, PATHSMC, stream_x2_sw,  accum_x2_sw  );
	accumSW( STEPSMC, PATHSMC, stream_x3_sw,  accum_x3_sw  );
	accumSW( STEPSMC, PATHSMC, stream_x4_sw,  accum_x4_sw  );
	accumSW( STEPSMC, PATHSMC, stream_y_sw,   accum_y_sw   );
	accumSW( STEPSMC, PATHSMC, stream_yx_sw,  accum_yx_sw  );
	accumSW( STEPSMC, PATHSMC, stream_yx2_sw, accum_yx2_sw );

	// ------------------------------------------------------------------------------------
	// CHECK VECTOR X
	// ------------------------------------------------------------------------------------
	printf("stock_hw.size() after  = %d\n",stock_hw.size());
	printf("stock_sw.size() after  = %d\n",stock_sw.size());
	printf("cashFlow_hw.size() after = %d\n",cashFlow_hw.size());
	printf("cashFlow_sw.size() after = %d\n",cashFlow_sw.size());
	printf("\n");
	printf("accum_x0_sw.size()  after = %d\n",accum_x0_sw.size());
	printf("accum_x1_sw.size()  after = %d\n",accum_x1_sw.size());
	printf("accum_x2_sw.size()  after = %d\n",accum_x2_sw.size());
	printf("accum_x3_sw.size()  after = %d\n",accum_x3_sw.size());
	printf("accum_x4_sw.size()  after = %d\n",accum_x4_sw.size());
	printf("accum_y_sw.size()   after = %d\n",accum_y_sw.size());
	printf("accum_yx_sw.size()  after = %d\n",accum_yx_sw.size());
	printf("accum_yx2_sw.size() after = %d\n",accum_yx2_sw.size());
	printf("\n");
	printf("accum_x0_sw.size()  after = %d\n",accum_x0_sw.size());
	printf("accum_x1_sw.size()  after = %d\n",accum_x1_sw.size());
	printf("accum_x2_sw.size()  after = %d\n",accum_x2_sw.size());
	printf("accum_x3_sw.size()  after = %d\n",accum_x3_sw.size());
	printf("accum_x4_sw.size()  after = %d\n",accum_x4_sw.size());
	printf("accum_y_sw.size()   after = %d\n",accum_y_sw.size());
	printf("accum_yx_sw.size()  after = %d\n",accum_yx_sw.size());
	printf("accum_yx2_sw.size() after = %d\n",accum_yx2_sw.size());
	printf("\n");
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_x0w\n");
	int errorCount_accum_x0 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_x0_hw.read();
		float sum_sw = accum_x0_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_x0++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_x1w\n");
	int errorCount_accum_x1 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_x1_hw.read();
		float sum_sw = accum_x1_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_x1++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_x2w\n");
	int errorCount_accum_x2 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_x2_hw.read();
		float sum_sw = accum_x2_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_x2++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_x3w\n");
	int errorCount_accum_x3 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_x3_hw.read();
		float sum_sw = accum_x3_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_x3++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_x4w\n");
	int errorCount_accum_x4 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_x4_hw.read();
		float sum_sw = accum_x4_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_x4++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_yw\n");
	int errorCount_accum_y = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_y_hw.read();
		float sum_sw = accum_y_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_y++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_yxw\n");
	int errorCount_accum_yx = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_yx_hw.read();
		float sum_sw = accum_yx_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_yx++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking accum_yx2w\n");
	int errorCount_accum_yx2 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = accum_yx2_hw.read();
		float sum_sw = accum_yx2_sw.read();
		if(sum_hw != sum_sw)
			errorCount_accum_yx2++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");

	if( (errorCount_accum_x0 !=0) || (errorCount_accum_x1 !=0) || (errorCount_accum_x2 !=0) || (errorCount_accum_x3 !=0) || (errorCount_accum_x4 !=0) || (errorCount_accum_y !=0) || (errorCount_accum_yx !=0) || (errorCount_accum_yx2 !=0) )
	{
		printf("\nTESTBENCH:  ERROR\n\n");
		return 1;
	}
	else
	{
		printf("\nTESTBENCH:  OK\n\n");
		return 0;
	}
}
