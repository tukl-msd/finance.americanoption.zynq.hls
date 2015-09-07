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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <hls_math.h>


//#define BLOCK_SIZE 8192  //x3 = 24576

#define STEPSMC 5 //10 //365

#define PATHSMC 1000 //1000 //5000

int main()
{
	float m1 = -1.25;
	float m2 = 0.37;
	float S0 = 50.0;

	hls::stream<float> in;
	hls::stream<float> stock;

	float vector_in[PATHSMC][STEPSMC];
	float stocksAll[PATHSMC][STEPSMC+1];

	printf("--------------------------------------------\n");
	printf("Generating Stimuli ...\n");
	for (int j = 0; j < STEPSMC; ++j)
	{
		for (int i = 0; i < PATHSMC; ++i)
		{
			float randnValue = rand()%10;///10000000000.0;

			vector_in[i][j] = randnValue;
			in.write(randnValue);
		}
	}

	for (int j = STEPSMC-1; j >=0; --j)
		for (int i = (PATHSMC-1); i >=0; --i)
			in.write(vector_in[i][j]);

	printf("--------------------------------------------\n");
	printf("running BSflex ...\n");

	//hls::stream<float> stockDebug;
	volatile uint32_t peekStep;
	volatile uint32_t peekPath;

	bsflexregio (	STEPSMC,
					PATHSMC,
					S0,
					m1,
					m2,
					&peekStep,
					&peekPath,
					in,
					stock );
					//stockDebug );

	// -----------------------------------------
	// SW
	// -----------------------------------------
	float prevStock [PATHSMC];
	for (int i = 0; i < PATHSMC; ++i)
	{
		prevStock[i]    = S0;
		stocksAll[i][0] = S0;
	}

	for(int j=0; j < STEPSMC; ++j)
	{
		for (int i = 0; i < PATHSMC; ++i)
		{
			float r1 = vector_in[i][j];
			float temp = m1 + m2 * r1;
			float exponent = hls::expf(temp);
			float swStock = prevStock[i] * exponent;

			prevStock[i] = swStock;

			stocksAll[i][j+1] = swStock;
		}
	}

	printf("--------------------------------------------\n");
	printf("checking values ...\n");

	if (stock.size() != PATHSMC*(STEPSMC+1))
	{
		printf("ERROR: wrong out.size() value:\n");
		printf("out.size() = %d  vs  PATHSMC*(STEPSMC+1) = %d\n",stock.size(),PATHSMC*(STEPSMC+1));
		return -1;
	}

	// full check
	int errorCount = 0;

	for(int j=STEPSMC; j >=0; --j)
	{
		for (int i = PATHSMC-1; i >=0; --i)
		{
			float hwStock = stock.read();

			float swStock = stocksAll[i][j];

			if ( hwStock != swStock)
			{
				float diff = hwStock - swStock;

				if( fabsf(diff) > 5.0e-3 )
				{
					printf("ERROR Rev: @(i,j)=(%5d,%4d) -- hwStock = %.3f  vs  swStock = %.3f >> diff = %.3e\n",i,j,hwStock,swStock,diff);
					errorCount++;
				}
			}

		}
	}

	if(errorCount == 0 )
	{
		printf("Testbench OK\n");
		return 0;
	}
	else
	{
		printf("Testbench ERROR\n");
		return -1;
	}
}

/*
	//float prevStock [PATHSMC];
	for (int i = 0; i < PATHSMC; ++i)
		prevStock [i] = S0;

	// forward check
	int errorCountFwd = 0;
	for(int j=0; j < STEPSMC; ++j)
	{
		for (int i = 0; i < PATHSMC; ++i)
		{
			//float hwStock = stockDebug.read();

			float r1 = vector_in[i][j];
			float temp = m1 + m2 * r1;
			float exponent = hls::expf(temp);
			float swStock = prevStock[i] * exponent;

			prevStock[i] = swStock;

			//if ( hwStock != swStock)
			//{
			//	printf("ERROR Debug Fwd: @(i,j)=(%5d,%4d) -- hwStock = %.3f  vs  swStock = %.3f Stock\n",i,j,hwStock,swStock);
			//	errorCountFwd++;
			//}

			// CHECK MATURITY (only output from BS in forward mode)
			if(j == (STEPSMC-1))
			{
				float hwStock = stock.read();

				if ( hwStock != swStock)
				{
					printf("ERROR Fwd: @(i,j)=(Maturity,%4d) -- hwStock = %.3f  vs  swStock = %.3f Stock\n",j,hwStock,swStock);
					errorCountFwd++;
				}
			}
		}
	}

	if(errorCountFwd == 0)
		printf("BSflex forward process OK\n");

	// backward check
	int errorCountRev = 0;

	for(int j=STEPSMC-1; j >=0; --j)
	{
		for (int i = 0; i < PATHSMC; ++i)
		{
			float hwStock = stock.read();

			float r1 = vector_in[i][j];
			float temp = m1 + m2 * r1;
			float exponent = hls::expf(-temp);
			float swStock = prevStock[i] * exponent;

			prevStock[i] = swStock;

			if ( hwStock != swStock)
			{
				printf("ERROR Rev: @(i,j)=(%5d,%4d) -- hwStock = %.3f  vs  swStock = %.3f Stock\n",i,j,hwStock,swStock);
				errorCountRev++;
			}

			//hwStock = stockDebug.read();
			//if ( hwStock != swStock)
			//{
			//	printf("ERROR Debug Rev: @(i,j)=(%5d,%4d) -- hwStock = %.3f  vs  swStock = %.3f Stock\n",i,j,hwStock,swStock);
			//	errorCountRev++;
			//}
		}
	}

	if(errorCountRev == 0)
		printf("BSflex backward process OK\n");

	if(errorCountFwd == 0 && errorCountRev == 0)
	{
		printf("Testbench OK\n");
		return 0;
	}
	else
	{
		printf("Testbench ERROR\n");
		return -1;
	}
}
*/
