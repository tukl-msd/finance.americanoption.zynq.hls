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


#include "correlator_hls.hpp"

void corramountregio (	const uint32_t amount,
						const float rho,
						const float sqrt_one_minus_rho2,
						volatile uint32_t *peekAmount,
						hls::stream<float> &rn_in_1,
						hls::stream<float> &rn_in_2,
						hls::stream<float> &rn_out_1,
						hls::stream<float> &rn_out_2)
{

#pragma HLS INTERFACE s_axilite port=return bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=amount bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=rho bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=sqrt_one_minus_rho2 bundle=CONTROL
#pragma HLS INTERFACE s_axilite port=peekAmount bundle=CONTROL

#pragma HLS INTERFACE axis register port=rn_in_1
#pragma HLS INTERFACE axis register port=rn_in_2
#pragma HLS INTERFACE axis register port=rn_out_1
#pragma HLS INTERFACE axis register port=rn_out_2


	for (uint32_t i = 0; i < amount; ++i)
	{
#pragma HLS PIPELINE II=1 enable_flush

		float r1 = rn_in_1.read();
		float r2 = rn_in_2.read();

		float y1 = rho*r1 + r2*sqrt_one_minus_rho2;

		rn_out_1.write(r1);
		rn_out_2.write(y1);

		*peekAmount = i;
	}

	return;
}
