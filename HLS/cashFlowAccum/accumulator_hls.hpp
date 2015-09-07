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


#ifndef __ACCUMULATOR_HLS_HPP__
#define __ACCUMULATOR_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

#define NELEMENTS 10

void cflowaccumregio(	const uint32_t pathsMC,
						float *totalSum,
						hls::stream<float> &inData );

#endif
