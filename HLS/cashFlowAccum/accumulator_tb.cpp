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
#define PATHSMC 1000

//int main(int argc, char *argv[]) {
int main() {


	// ------------------------------------------------------------------------------------
	// PREPARE
	// ------------------------------------------------------------------------------------
	float sums[NELEMENTS];

	hls::stream<float> inData_hw;
#pragma HLS stream depth=1000 variable=inData_hw

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

	float totalSum_sw = (float) 0.0f;

	for(uint8_t i=0; i<NELEMENTS;++i)
		totalSum_sw += sums[i];

	printf("inData_hw.size() before = %d\n",inData_hw.size());
	// ------------------------------------------------------------------------------------
	// HW
	// ------------------------------------------------------------------------------------
	float totalSum_hw;

	cflowaccumregio(	PATHSMC,
						&totalSum_hw,
						inData_hw  );

	// ------------------------------------------------------------------------------------
	// CHECK VECTOR X
	// ------------------------------------------------------------------------------------
	printf("inData_hw.size() after  = %d\n",inData_hw.size());

	printf("----------------------------------------------------------------------------\n");
	printf("\n");

	int errorCount = 0;

	if(totalSum_hw != totalSum_sw)
		errorCount++;
	printf("%s: >> sum_sw = %.3f  vs  sum_hw = %.3f\n",(totalSum_hw==totalSum_sw)?"-----":"ERROR",totalSum_sw,totalSum_hw);

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
