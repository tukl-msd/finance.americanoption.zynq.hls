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

#include "maxsplit_hls.hpp"

#include <stdint.h>
#include <stdio.h>
#include <hls_math.h>

#include <hls_stream.h>

#define STEPSMC 5
#define PATHSMC 20

int main ()
{

	//hls::stream<float> stock1_sw;
	hls::stream<float> stock1_hw;

	//hls::stream<float> stock2_sw;
	hls::stream<float> stock2_hw;

	hls::stream<float> outmax_sw;
	hls::stream<float> outmax_hw;

	hls::stream<float> outmaxFIFO_sw;
	hls::stream<float> outmaxFIFO_hw;


	// -------------------------------------------------------------
	// SW
	// -------------------------------------------------------------
	printf("======================================================\n");
	printf("CHECK\n");
	printf("\n");

	float S0 = 100.0f;
	const float Srange = 5.0f;
	const float Sincr = 2*Srange / (float) PATHSMC;

	for(uint32_t step=0; step<=STEPSMC; ++step)
	{
		for(uint32_t path=0; path<PATHSMC; ++path)
		{
			float s1 = S0 + (float) ( (-Srange + Sincr * (float)path) * (float) (STEPSMC-step));
			float s2 = S0 + (float) ( ( Srange - Sincr * (float)path) * (float) (STEPSMC-step));

			stock1_hw.write( s1 );
			stock2_hw.write( s2 );

			float maxValue = fmaxf(s1,s2);

			outmaxFIFO_sw.write( maxValue );

			if(step > 0)
				outmax_sw.write( maxValue );

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

	maxsplitregio (	STEPSMC,
					PATHSMC,
					&peekStep,
					&peekPath,
					stock1_hw,
					stock2_hw,
					outmax_hw,
					outmaxFIFO_hw );

	// -------------------------------------------------------------
	// CHECK
	// -------------------------------------------------------------

	printf("outmaxFIFO_hw.size() = %d\n",outmaxFIFO_hw.size());
	printf("outmaxFIFO_sw.size() = %d\n",outmaxFIFO_sw.size());
	printf("outmax_hw.size() = %d\n",outmax_hw.size());
	printf("outmax_sw.size() = %d\n",outmax_sw.size());

	if(outmaxFIFO_hw.size() != ((STEPSMC+1)*PATHSMC))
	{
		printf("ERROR: outmaxFIFO_hw.size()\n");
		return 1;
	}
	if(outmax_hw.size() != ((STEPSMC)*PATHSMC))
	{
		printf("ERROR: outmax_hw.size()\n");
		return 1;
	}

	int errorCount = 0;

	// TEST outmax
	printf("--------------------------------------------------------\n");
	printf("TEST outmax\n");
	for(uint32_t step=1; step<=STEPSMC; ++step)
	{
		printf("\n");
		for(uint32_t path=0; path<PATHSMC; ++path)
		{
			float max_sw = outmax_sw.read();
			float max_hw = outmax_hw.read();

			if( (max_sw != max_hw) )
			{
				errorCount++;
			}

			printf("%s:@step = %3d, path = %3d >> max_sw = %.3f  vs  max_hw = %.3f\n",(max_sw==max_hw)?"-----":"ERROR",step,path,max_sw,max_hw);
		}
	}

	// TEST outmaxFIFO
	printf("--------------------------------------------------------\n");
	printf("TEST outmaxFIFO\n");
	for(uint32_t step=0; step<=STEPSMC; ++step)
	{
		printf("\n");
		for(uint32_t path=0; path<PATHSMC; ++path)
		{
			float max_sw = outmaxFIFO_sw.read();
			float max_hw = outmaxFIFO_hw.read();

			if( (max_sw != max_hw) )
			{
				errorCount++;
			}

			printf("%s:@step = %3d, path = %3d >> max_sw = %.3f  vs  max_hw = %.3f\n",(max_sw==max_hw)?"-----":"ERROR",step,path,max_sw,max_hw);
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
