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

#ifndef __LSUPDATE2_HLS_HPP__
#define __LSUPDATE2_HLS_HPP__

#include <stdint.h>
#include <hls_stream.h>

#define CASHFLOW_SIZE 32768

void lsupdate2regio ( 	const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float discount,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &contin_in,
						hls::stream<float> &payoff_in,
						//hls::stream<float> &cashFlow_in,
						//hls::stream<float> &cashFlow_out,
						hls::stream<float> &cashFlowDisc_out,
						hls::stream<float> &toAccum_out );

#endif
