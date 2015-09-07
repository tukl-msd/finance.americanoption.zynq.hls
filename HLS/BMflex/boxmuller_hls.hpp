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

#ifndef __BOXMULLER_HLS_HPP__
#define __BOXMULLER_HLS_HPP__

#include <stdint.h>
#include <hls_stream.h>

#define PI 3.141592653589793f
#define PIx2 6.283185307179586f

#define UINT2FLOAT 4294967295.0f

#define UINT2FLOAT_CORRECTION 4294967296.0f

#define COEF1 (1/UINT2FLOAT)
#define COEF2 (PIx2/UINT2FLOAT)


void bmflexregio (	const uint32_t amount,
					const uint32_t amount_div_2,
					volatile uint32_t *peekAmount,
					hls::stream<uint32_t> &uniform,
					hls::stream<float> &normal);

#endif
