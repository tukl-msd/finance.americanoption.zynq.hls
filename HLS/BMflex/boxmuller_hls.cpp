/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier Alejandro Varela (varela@eit.uni-kl.de)
//
// Last review: 29/06/2015
//
// Using: Xilinx Vivado HLS 2014.3
//
///////////////////////////////////////////////////  

#include "boxmuller_hls.hpp"

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <hls_stream.h>
#include <hls_math.h>


void bmflexregio (	const uint32_t amount,
					const uint32_t amount_div_2,
					volatile uint32_t *peekAmount,
					hls::stream<uint32_t> &uniform,
					hls::stream<float> &normal)
{


#pragma HLS INTERFACE axis register port=normal
#pragma HLS INTERFACE axis register port=uniform

#pragma HLS INTERFACE s_axilite port=amount bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=amount_div_2 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekAmount bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL


	for (uint32_t i=0; i < amount; ++i)
	{
#pragma HLS PIPELINE II=2 enable_flush


		// read uniform numbers in a given sequence
		float temp1 = ((float) uniform.read()) / ((float) UINT2FLOAT);
		float temp2 = ((float) uniform.read()) / ((float) UINT2FLOAT);

		// correct the numbers (sequence here is irrelevant)
		float in1corr = 1.0f + temp1;
		float in2corr = 1.0f + temp2;

#pragma HLS RESOURCE variable=in1corr core=FAddSub_nodsp
#pragma HLS RESOURCE variable=in2corr core=FAddSub_nodsp

		float corrected_in1;
		float corrected_in2;

		if(temp1 < 0)
			corrected_in1 = in1corr;
		else
			corrected_in1 = temp1;

		if(temp2 < 0)
			corrected_in2 = in2corr;
		else
			corrected_in2 = temp2;

		// reassign the correct input sequence (forward / reverse)
		float in1, in2;

		if(i < amount_div_2)
		{
			in1 = corrected_in1;	// Fwd
			in2 = corrected_in2;	// Fwd
		}
		else
		{
			in1 = corrected_in2;	// Rev
			in2 = corrected_in1;	// Rev
		}

		// BoxMuller
		float u1 = in1;  					//[0,1]-real-interval
		float u2 = in2 * (float) PIx2;  	//[0,1]-real-interval
#pragma HLS RESOURCE variable=u2 core=FMul_nodsp

		float logf_u1 = logf(u1);
#pragma HLS RESOURCE variable=logf_u1 core=FLog_nodsp

		float logf_u1_x2 = logf_u1 * ((float) 2.0f);
#pragma HLS RESOURCE variable=logf_u1_x2 core=FMul_nodsp

    	float sqrtu1 = sqrtf(-logf_u1_x2);
#pragma HLS RESOURCE variable=sqrtu1 core=FSqrt


    	//float arg = ((float) PIx2) * u2;
		//#pragma HLS RESOURCE variable=logf_u1_x2 core=FMul_nodsp // meddsp // maxdsp // fulldsp
    	float arg = u2;

    	// -------------------------------------------------
    	float cosf_arg = cosf(arg);

    	float z0 = sqrtu1 * cosf_arg;
#pragma HLS RESOURCE variable=z0 core=FMul_nodsp

    	// -------------------------------------------------
    	float sinf_arg = sinf(arg);

    	float z1 = sqrtu1 * sinf_arg;
#pragma HLS RESOURCE variable=z1 core=FMul_nodsp

    	// -------------------------------------------------
    	// reassign the correct output sequence (forward / reverse)
    	float out0;
    	float out1;

    	if(i < amount_div_2)
		{
			out0 = z0;	// Fwd
			out1 = z1;	// Fwd
		}
		else
		{
			out0 = z1;	// Fwd
			out1 = z0;	// Fwd
		}

    	normal.write(out0);
		normal.write(out1);

	}

    return;
}


