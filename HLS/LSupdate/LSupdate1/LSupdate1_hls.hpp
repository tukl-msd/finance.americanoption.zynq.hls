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

#ifndef __LSUPDATE1_HLS_HPP__
#define __LSUPDATE1_HLS_HPP__

#include <stdint.h>
#include <hls_stream.h>

void lsupdate1regio ( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float K,
						const uint32_t callPut,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &stock,
						hls::stream<float> &b0_in,
						hls::stream<float> &b1_in,
						hls::stream<float> &b2_in,
						hls::stream<float> &contin_out,
						hls::stream<float> &payoff_out );

#endif
