/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier A. Varela (varela@eit.uni-kl.de)
//
// Last review: 30/06/2015
//
// Zynq ZC702
//
/////////////////////////////////////////////////// 

#ifndef __LS_GOLDEN_H__
#define __LS_GOLDEN_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "mt19937ar.h"

const float PI = 3.1415927;

const int MAXSTEPS = 1000;
const int MAXPATHS = 32e3;


float ls_golden ( 	const uint32_t stepsMC,
					const uint32_t pathsMC,
					const float S0_1,
					const float S0_2,
					const int callPut,  // callPut = 1 (call); callPut = -1 (put)
					const float rho,
                    const float sqrt_1_minus_rho2,
                    const float K,
					const float discount,
                    const float m11,
                    const float m12,
                    const float m21,
					const float m22,
					const uint32_t seedValue1,
					const uint32_t seedValue2 );
					
					
float ls_goldenAccum ( 	const uint32_t stepsMC,
					const uint32_t pathsMC,
					const float S0_1,
					const float S0_2,
					const int callPut,  // callPut = 1 (call); callPut = -1 (put)
					const float rho,
                    const float sqrt_1_minus_rho2,
                    const float K,
					const float discount,
                    const float m11,
                    const float m12,
                    const float m21,
					const float m22,
					const uint32_t seedValue1,
					const uint32_t seedValue2 );
					
float ls_goldenNoMalloc ( 	const uint32_t stepsMC,
					const uint32_t pathsMC,
					const float S0_1,
					const float S0_2,
					const int callPut,  // callPut = 1 (call); callPut = -1 (put)
					const float rho,
                    const float sqrt_1_minus_rho2,
                    const float K,
					const float discount,
                    const float m11,
                    const float m12,
                    const float m21,
					const float m22,
					const uint32_t seedValue1,
					const uint32_t seedValue2 );

#endif