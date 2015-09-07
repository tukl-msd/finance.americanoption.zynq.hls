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

#include <hls_math.h>
#include <hls_stream.h>

void maxsplitregio (	const uint32_t stepsMC,
						const uint32_t pathsMC,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &in1,
						hls::stream<float> &in2,
						hls::stream<float> &outmax,
						hls::stream<float> &outmaxFIFO )
{
#pragma HLS INTERFACE s_axilite port=stepsMC 	bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=pathsMC 	bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekStep 	bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekPath 	bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=return 	bundle=CONTROL

#pragma HLS INTERFACE axis register port=in1
#pragma HLS INTERFACE axis register port=in2
#pragma HLS INTERFACE axis register port=outmax
#pragma HLS INTERFACE axis register port=outmaxFIFO

	*peekStep = 0xFFFFFFFF;
	*peekPath = 0xFFFFFFFF;

	stepsLoop:for(uint32_t step=0; step<=stepsMC; ++step)
	{
		pathsLoop:for(uint32_t path=0; path<pathsMC; ++path)
		{
#pragma HLS PIPELINE II=1 enable_flush

			float inValue1 = in1.read();
			float inValue2 = in2.read();

			float maxValue = fmaxf(inValue1, inValue2);


			outmaxFIFO.write( maxValue );

			if(step > 0)
				outmax.write( maxValue );

			*peekStep = step;
			*peekPath = path;
		}
	}
	return;
}
