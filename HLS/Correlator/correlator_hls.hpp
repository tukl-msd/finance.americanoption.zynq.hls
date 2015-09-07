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

#ifndef __CORRELATOR_HLS_HPP__
#define __CORRELATOR_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

void corramountregio (	const uint32_t amount,
						const float rho,
						const float sqrt_one_minus_rho2,
						volatile uint32_t *peekAmount,
						hls::stream<float> &rn_in_1,
						hls::stream<float> &rn_in_2,
						hls::stream<float> &rn_out_1,
						hls::stream<float> &rn_out_2);

#endif
