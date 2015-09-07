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
#include <ap_int.h>
#include <stdint.h>

#define ACCUM_ELEM 16

void accumregio(	const uint32_t stepsMC,
					const uint32_t pathsMC,
					volatile uint32_t *peekStep,
					volatile uint32_t *peekPath,
					hls::stream<float> &inData,
					hls::stream<float> &outAccum );

#endif
