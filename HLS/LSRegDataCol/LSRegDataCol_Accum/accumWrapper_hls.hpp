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

#ifndef __ACCUMWRAPPER_HLS_HPP__
#define __ACCUMWRAPPER_HLS_HPP__

#include <hls_stream.h>
#include <stdint.h>

//#define NELEMENTS 4

void lsrdcaccumregio (	const uint32_t stepsMC,
						const uint32_t pathsMC,
						volatile uint32_t *peekStep_x0,
						volatile uint32_t *peekStep_x1,
						volatile uint32_t *peekStep_x2,
						volatile uint32_t *peekStep_x3,
						volatile uint32_t *peekStep_x4,
						volatile uint32_t *peekStep_y,
						volatile uint32_t *peekStep_yx,
						volatile uint32_t *peekStep_yx2,
						volatile uint32_t *peekPath_x0,
						volatile uint32_t *peekPath_x1,
						volatile uint32_t *peekPath_x2,
						volatile uint32_t *peekPath_x3,
						volatile uint32_t *peekPath_x4,
						volatile uint32_t *peekPath_y,
						volatile uint32_t *peekPath_yx,
						volatile uint32_t *peekPath_yx2,
						hls::stream<float> &stream_x0,
						hls::stream<float> &stream_x1,
						hls::stream<float> &stream_x2,
						hls::stream<float> &stream_x3,
						hls::stream<float> &stream_x4,
						hls::stream<float> &stream_y,
						hls::stream<float> &stream_yx,
						hls::stream<float> &stream_yx2,
						hls::stream<float> &accum_x0,
						hls::stream<float> &accum_x1,
						hls::stream<float> &accum_x2,
						hls::stream<float> &accum_x3,
						hls::stream<float> &accum_x4,
						hls::stream<float> &accum_y,
						hls::stream<float> &accum_yx,
						hls::stream<float> &accum_yx2 );
#endif
