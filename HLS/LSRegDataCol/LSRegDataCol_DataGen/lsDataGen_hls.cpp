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

void lsdatagenregio ( const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float K,
						const uint32_t callPut,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
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
#pragma HLS INTERFACE axis register port=stock
#pragma HLS INTERFACE axis register port=cashFlow

#pragma HLS INTERFACE axis register port=stream_x0
#pragma HLS INTERFACE axis register port=stream_x1
#pragma HLS INTERFACE axis register port=stream_x2
#pragma HLS INTERFACE axis register port=stream_x3
#pragma HLS INTERFACE axis register port=stream_x4
#pragma HLS INTERFACE axis register port=stream_y
#pragma HLS INTERFACE axis register port=stream_yx
#pragma HLS INTERFACE axis register port=stream_yx2

#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=callPut bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=K bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL


	*peekStep = 0xFFFFFFFF;
	*peekPath = 0xFFFFFFFF;

	stepsLoop:for(uint32_t step=0; step < stepsMC; ++step)
	{
		*peekStep = step;

		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
//#pragma HLS PIPELINE II=1 enable_flush
#pragma HLS PIPELINE II=1

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



			//*peekStep = step;
			*peekPath = path;
		}
	}
	return;
}
