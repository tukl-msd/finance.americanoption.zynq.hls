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

#include <stdio.h>
//-----------------------
#include <hls_stream.h>
#include <stdlib.h>
#include <stdint.h>
//-----------------------

#define STEPSMC 10
#define PATHSMC 100

//int main(int argc, char *argv[]) {
int main() {


	// ------------------------------------------------------------------------------------
	// PREPARE
	// ------------------------------------------------------------------------------------
	float sums[NELEMENTS];

	hls::stream<float> inData_hw;
#pragma HLS stream depth=1000 variable=inData_hw

	hls::stream<float> outData_hw;
#pragma HLS stream depth=1000 variable=outData_hw

	hls::stream<float> outData_sw;
#pragma HLS stream depth=1000 variable=outData_sw

	for(int step=0; step<STEPSMC; ++step)
	{
		for(uint8_t i=0; i<NELEMENTS; ++i)
			sums[i] = (float) 0.0f;

		uint8_t index = (uint8_t) 0;

		for(int path=0; path<PATHSMC; ++path)
		{
			float temp = (float) ((float) (rand()%1000000))/100.0f;// + ( ((float) (rand()%100))/100.0f );//%10.0f; //((float) (step+path))
			sums[index] += temp;
			index = (index<(NELEMENTS-1))?++index:(uint8_t)0;
			inData_hw.write(temp);
		}

		float totalSum = (float) 0.0f;

		for(uint8_t i=0; i<NELEMENTS;++i)
			totalSum += sums[i];

		outData_sw.write(totalSum);
	}

	printf("inData_hw.size() before = %d\n",inData_hw.size());
	// ------------------------------------------------------------------------------------
	// HW
	// ------------------------------------------------------------------------------------

	accumregio(	STEPSMC,
				PATHSMC,
				inData_hw,
				outData_hw );

	// ------------------------------------------------------------------------------------
	// CHECK VECTOR X
	// ------------------------------------------------------------------------------------
	printf("inData_hw.size() after  = %d\n",inData_hw.size());
	printf("outData_hw.size() = %d\n",outData_hw.size());

	printf("----------------------------------------------------------------------------\n");
	printf("\n");

	int errorCount = 0;
	for(int step=0; step<STEPSMC; ++step)
	{
		float sum_hw = outData_hw.read();
		float sum_sw = outData_sw.read();
		if(sum_hw != sum_sw)
			errorCount++;
		printf("%s:@step = %3d >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(sum_sw==sum_hw)?"-----":"ERROR",step,sum_sw,sum_hw);
	}
	printf("----------------------------------------------------------------------------\n");

	if( (errorCount !=0) )
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
