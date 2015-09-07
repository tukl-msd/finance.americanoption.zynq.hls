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

#ifndef __BSFLEX_HLS_HPP__
#define __BSFLEX_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

void bsflexregio (	uint32_t stepsMC,
					uint32_t pathsMC,
					//uint32_t stepsMC_x2,
					float S0,
					float m1,
					float m2,
					volatile uint32_t *peekStep,
					volatile uint32_t *peekPath,
					hls::stream<float> &rn_in,
					hls::stream<float> &stock );
					//hls::stream<float> &stockDebug );

#endif
