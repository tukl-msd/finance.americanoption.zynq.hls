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

#include "cholesky_hls.hpp"

#include <hls_math.h>

void choleskyregio (  	const uint32_t stepsMC,
						volatile float b0_vector[CHOLESKY_VECTOR_SIZE],
						volatile float b1_vector[CHOLESKY_VECTOR_SIZE],
						volatile float b2_vector[CHOLESKY_VECTOR_SIZE],
						volatile uint32_t *peekStep,
						hls::stream<float> &in_one,
						hls::stream<float> &in_x,
						hls::stream<float> &in_x2,
						hls::stream<float> &in_x3,
						hls::stream<float> &in_x4,
						hls::stream<float> &in_y,
						hls::stream<float> &in_yx,
						hls::stream<float> &in_yx2,
						hls::stream<float> &out_b0,
						hls::stream<float> &out_b1,
						hls::stream<float> &out_b2 )
{

#pragma HLS INTERFACE axis register port=in_one
#pragma HLS INTERFACE axis register port=in_x
#pragma HLS INTERFACE axis register port=in_x2
#pragma HLS INTERFACE axis register port=in_x3
#pragma HLS INTERFACE axis register port=in_x4
#pragma HLS INTERFACE axis register port=in_y
#pragma HLS INTERFACE axis register port=in_yx
#pragma HLS INTERFACE axis register port=in_yx2

#pragma HLS INTERFACE axis register port=out_b0
#pragma HLS INTERFACE axis register port=out_b1
#pragma HLS INTERFACE axis register port=out_b2

#pragma HLS INTERFACE s_axilite port=peekStep bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=b0_vector bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=b1_vector bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=b2_vector bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=stepsMC bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL

	*peekStep = 0xFFFFFFFF;

	cleanLoop:for(uint32_t index=0; index<CHOLESKY_VECTOR_SIZE; ++index)
	{
#pragma HLS PIPELINE II=1 enable_flush
		b0_vector[index] = (float) 123456789.0f;
		b1_vector[index] = (float) 123456789.0f;
		b2_vector[index] = (float) 123456789.0f;
	}

	*peekStep = 0xEEEEEEEE;

	mainLoop:for(uint32_t step=1; step<=stepsMC; ++step)
	{
//#pragma HLS PIPELINE II=1 enable_flush
		
		float one  = in_one.read();
		float x    = in_x.read();
		float x2   = in_x2.read();
		float x3   = in_x3.read();
		float x4   = in_x4.read();
		float y    = in_y.read();
		float yx   = in_yx.read();
		float yx2  = in_yx2.read();
			
		// Cholesky 3x3
		//% one
		float l11 = sqrtf(one);
//#pragma HLS RESOURCE variable=l11 core=FSqrt

		//%two
		float l21 = x/l11;
		float l31 = x2/l11;
		//%three
		float temp1 = l21*l21;
//#pragma HLS RESOURCE variable=temp1 core=FMul_nodsp
		float temp2 = x2-temp1;
//#pragma HLS RESOURCE variable=temp2 core=FAddSub_fulldsp
		float l22 = sqrtf(temp2);
//#pragma HLS RESOURCE variable=l22 core=FSqrt

		float temp3 = l21*l31;
//#pragma HLS RESOURCE variable=temp3 core=FMul_nodsp
		float temp4 = x3-temp3;
//#pragma HLS RESOURCE variable=temp4 core=FAddSub_fulldsp
		float l32 = (temp4)/l22;
		//float l22 = sqrtf(x2-l21*l21);
		//float l32 = (x3-l21*l31)/l22;
		//%four
		float l33 = sqrtf((x4-l31*l31)-l32*l32);
		
		//% Forward substitution
		float z1  = y /l11;
		float z2  = (yx-l21*z1)/l22;
		float z3  = (yx2-l31*z1-l32*z2)/l33;
		//% Backward substitution
		float b2  = z3 / l33;
		float b1  = (z2 - l32*b2) / l22;
		float b0  = (z1 - l21*b1 - l31*b2)/l11;  
		
		out_b0.write(b0);
		out_b1.write(b1);
		out_b2.write(b2);

		//int index = stepsMC-step;
		uint32_t index = stepsMC-step;
		if(index < CHOLESKY_VECTOR_SIZE)
		{
			b0_vector[index] = b0;
			b1_vector[index] = b1;
			b2_vector[index] = b2;
		}

		*peekStep = step;
	}
	
	return;
}
