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

#ifndef __MAXSPLIT_HLS_HPP__
#define __MAXSPLIT_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

void maxsplitregio (	const uint32_t stepsMC,
						const uint32_t pathsMC,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &in1,
						hls::stream<float> &in2,
						hls::stream<float> &outmax,
						hls::stream<float> &outmaxFIFO );


#endif
