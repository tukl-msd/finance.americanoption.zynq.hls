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

#ifndef __LSDATAGEN_HLS_HPP__
#define __LSDATAGEN_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

void lsdatagenregio ( const uint32_t stepsMC,
						const uint32_t pathsMC,
						const float K,
						const uint32_t callPut,
						volatile uint32_t *peekStep,
						volatile uint32_t *peekPath,
						hls::stream<float> &stock,
						hls::stream<float> &cashFlow,
						hls::stream<float> &stream_x0,
						hls::stream<float> &stream_x1,
						hls::stream<float> &stream_x2,
						hls::stream<float> &stream_x3,
						hls::stream<float> &stream_x4,
						hls::stream<float> &stream_y,
						hls::stream<float> &stream_yx,
						hls::stream<float> &stream_yx2 );

#endif
