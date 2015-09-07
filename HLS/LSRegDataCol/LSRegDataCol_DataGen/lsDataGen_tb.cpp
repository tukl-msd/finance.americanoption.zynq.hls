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

#include "lsDataGen_hls.hpp"

#include <stdio.h>
//-----------------------
#include <hls_stream.h>
#include <hls_math.h>
#include <stdlib.h>
#include <stdint.h>
//-----------------------

#define STEPSMC 10
#define PATHSMC 100

/*
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
*/

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
			float cflow = cashFlow.read();

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
			float y;

			if(inTheMoney == true)
			{
				x0 = (float) 1.0f;
				x1 = (float) s;
				x2 = (float) s2;
				y  = (float) cflow;
			}
			else
			{
				x0 = (float) 0.0f;
				x1 = (float) 0.0f;
				x2 = (float) 0.0f;
				y  = (float) 0.0f;
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
	//float sums[ACCUM_ELEM];

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
/*
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
*/
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

	// ------------------------------------------------------------------------------------
	// HW
	// ------------------------------------------------------------------------------------
	volatile uint32_t peekStep;
	volatile uint32_t peekPath;

	lsdatagenregio ( 	STEPSMC,
						PATHSMC,
						K,
						callPut,
						&peekStep,
						&peekPath,
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
	// ------------------------------------------------------------------------------------
	// SW
	// ------------------------------------------------------------------------------------
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
	// CHECK VECTOR X
	// ------------------------------------------------------------------------------------
	printf("stock_hw.size() after  = %d\n",stock_hw.size());
	printf("stock_sw.size() after  = %d\n",stock_sw.size());
	printf("cashFlow_hw.size() after = %d\n",cashFlow_hw.size());
	printf("cashFlow_sw.size() after = %d\n",cashFlow_sw.size());
	printf("\n");
	printf("stream_x0_sw.size()  after = %d\n",stream_x0_sw.size());
	printf("stream_x1_sw.size()  after = %d\n",stream_x1_sw.size());
	printf("stream_x2_sw.size()  after = %d\n",stream_x2_sw.size());
	printf("stream_x3_sw.size()  after = %d\n",stream_x3_sw.size());
	printf("stream_x4_sw.size()  after = %d\n",stream_x4_sw.size());
	printf("stream_y_sw.size()   after = %d\n",stream_y_sw.size());
	printf("stream_yx_sw.size()  after = %d\n",stream_yx_sw.size());
	printf("stream_yx2_sw.size() after = %d\n",stream_yx2_sw.size());
	printf("\n");
	printf("stream_x0_sw.size()  after = %d\n",stream_x0_sw.size());
	printf("stream_x1_sw.size()  after = %d\n",stream_x1_sw.size());
	printf("stream_x2_sw.size()  after = %d\n",stream_x2_sw.size());
	printf("stream_x3_sw.size()  after = %d\n",stream_x3_sw.size());
	printf("stream_x4_sw.size()  after = %d\n",stream_x4_sw.size());
	printf("stream_y_sw.size()   after = %d\n",stream_y_sw.size());
	printf("stream_yx_sw.size()  after = %d\n",stream_yx_sw.size());
	printf("stream_yx2_sw.size() after = %d\n",stream_yx2_sw.size());
	printf("\n");
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_x0\n");
	int errorCount_stream_x0 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_x0_hw.read();
			float value_sw = stream_x0_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_x0++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_x1\n");
	int errorCount_stream_x1 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_x1_hw.read();
			float value_sw = stream_x1_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_x1++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_x2\n");
	int errorCount_stream_x2 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_x2_hw.read();
			float value_sw = stream_x2_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_x2++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_x3\n");
	int errorCount_stream_x3 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_x3_hw.read();
			float value_sw = stream_x3_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_x3++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_x4\n");
	int errorCount_stream_x4 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_x4_hw.read();
			float value_sw = stream_x4_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_x4++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_y\n");
	int errorCount_stream_y = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_y_hw.read();
			float value_sw = stream_y_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_y++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_yx\n");
	int errorCount_stream_yx = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_yx_hw.read();
			float value_sw = stream_yx_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_yx++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");
	printf("checking stream_yx2\n");
	int errorCount_stream_yx2 = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		for(int path=0; path<PATHSMC; ++path)
		{
			float value_hw = stream_yx2_hw.read();
			float value_sw = stream_yx2_sw.read();
			if(value_hw != value_sw)
				errorCount_stream_yx2++;
			printf("%s:@step = %3d, path = %3d >> value_sw = %.3f  vs  value_hw = %.3f\n",(value_sw==value_hw)?"-----":"ERROR",step,path,value_sw,value_hw);
		}
	}
	printf("----------------------------------------------------------------------------\n");

	if( (errorCount_stream_x0 !=0) || (errorCount_stream_x1 !=0) || (errorCount_stream_x2 !=0) || (errorCount_stream_x3 !=0) || (errorCount_stream_x4 !=0) || (errorCount_stream_y !=0) || (errorCount_stream_yx !=0) || (errorCount_stream_yx2 !=0) )
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
