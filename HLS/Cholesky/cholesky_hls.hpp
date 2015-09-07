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


#ifndef __CHOLESKY_HLS_HPP__
#define __CHOLESKY_HLS_HPP__

#include "cholesky_hls.hpp"

#include <hls_stream.h>
#include <stdint.h>

#define CHOLESKY_VECTOR_SIZE 1024

void choleskyregio ( const uint32_t stepsMC,
					volatile float b0_vector[CHOLESKY_VECTOR_SIZE],
					volatile float b1_vector[CHOLESKY_VECTOR_SIZE],
					volatile float b2_vector[CHOLESKY_VECTOR_SIZE],
					volatile uint32_t *peekStep,
					hls::stream<float> &in_one,
					hls::stream<float> &in_x,
					hls::stream<float> &in_x2,
					hls::stream<float> &in_x3,
					hls::stream<float> &in_x4,
					hls::stream<float> &in_y,
					hls::stream<float> &in_yx,
					hls::stream<float> &in_yx2,
					hls::stream<float> &out_b0,
					hls::stream<float> &out_b1,
					hls::stream<float> &out_b2 );

#endif
