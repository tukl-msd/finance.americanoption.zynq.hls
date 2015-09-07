/////////////////////////////////////////////////// 
// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Javier A. Varela (varela@eit.uni-kl.de)
//
// Last review: 30/06/2015
//
//
//
/////////////////////////////////////////////////// 

#include "ls_golden.h"

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
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
					const uint32_t seedValue2 )
{
	int normalIndex;
	int stockIndex;
	/*
	float normal1[MAXPATHS][MAXSTEPS];
	float normal2[MAXPATHS][MAXSTEPS];
	float stock1[MAXPATHS][MAXSTEPS+1];
	float stock2[MAXPATHS][MAXSTEPS+1];
	float lastS1[MAXPATHS];
	float lastS2[MAXPATHS];
	float cashFlow[MAXPATHS];
	*/
	
	// MEMORY ALLOCATION

	float *normal1 	= (float*) malloc( (stepsMC  ) * pathsMC * sizeof(float) );
	float *normal2 	= (float*) malloc( (stepsMC  ) * pathsMC * sizeof(float) );
	float *stock1 	= (float*) malloc( (stepsMC+1) * pathsMC * sizeof(float) );
	float *stock2 	= (float*) malloc( (stepsMC+1) * pathsMC * sizeof(float) );
	float *lastS1 	= (float*) malloc( pathsMC * sizeof(float) );
	float *lastS2 	= (float*) malloc( pathsMC * sizeof(float) );
	float *cashFlow = (float*) malloc( pathsMC * sizeof(float) );

	if( normal1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc normal1\n");
		return -1;
	}
	if( normal2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc normal2\n");
		return -1;
	}
	if( stock1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc stock1\n");
		return -1;
	}
	if( stock2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc stock2\n");
		return -1;
	}
    if( lastS1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc lastS1\n");
		return -1;
	}
	if( lastS2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc lastS2\n");
		return -1;
	}
	if( cashFlow == NULL )
	{
		printf("ERROR: ls_golden unable to malloc cashFlow\n");
		return -1;
	}

	// ----------------------------------------------------------
	// Check callPut value
	if( (callPut != 1) && (callPut != -1) )
	{
		printf("ERROR: ls_golden: callPut value can only be 1 (call) or -1 (put)\n");
		return -1;
	}

	// ----------------------------------------------------------
	// GENERATE RANDOM NUMBERS via Mersenne-Twister and BoxMuller
	// normal 1
	init_genrand(seedValue1);
	generate_numbers();
	
	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal1[normalIndex] = z0;
			++normalIndex;
			normal1[normalIndex] = z1;
			++normalIndex;
		}
	}

	// normal 2
	init_genrand(seedValue2);
	generate_numbers();
	
	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal2[normalIndex] = z0;
			++normalIndex;
			normal2[normalIndex] = z1;
			++normalIndex;
		}
	}

	// ----------------------------------------------------------
	// GENERATE PATHS via Black-Scholes
	normalIndex = 0;
	stockIndex  = 0;

	for (uint32_t step = 0; step <= stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1;
			float s2;

			// spot price
			if(step == 0)
			{
				s1 = S0_1;
				s2 = S0_2;
			}						
			else
			{
				// read normal numbers and correlate them with coefficient rho
				float n1 = normal1[normalIndex];
				float n2 = normal2[normalIndex];

				float z1 = n1;
				float z2 = rho*z1 + sqrt_1_minus_rho2 * n2;

				// Black-Scholes
				float prevS1 = lastS1[path];
				float prevS2 = lastS2[path];

				s1  = prevS1 * expf(m11 + m12 * z1);
				s2  = prevS2 * expf(m21 + m22 * z2);
				
				++normalIndex;
			}

			lastS1[path] = s1;
			lastS2[path] = s2;
			 
			stock1[stockIndex] = s1;
			stock2[stockIndex] = s2;

			++stockIndex;
		}
	}
	
	// ----------------------------------------------------------
    // CASH FLOW @ Maturity
	stockIndex = stepsMC;
	for (uint32_t path = 0; path < pathsMC; ++path)
	{
		float s1 = stock1[stockIndex];
		float s2 = stock2[stockIndex];
		float Smax = fmaxf(s1, s2);
		cashFlow[path] = fmaxf(callPut*(Smax-K), (float) 0.0f);
		++stockIndex;
	}

	// ----------------------------------------------------------
	// LONGSTAFF-SCHWARTZ
    for (int32_t step = (stepsMC-1); step >=0; --step)
	{
		stockIndex = step * pathsMC;

		float x0 = (float)0.0;
		float x1 = (float)0.0;
		float x2 = (float)0.0;
		float x3 = (float)0.0;
		float x4 = (float)0.0;
		float y  = (float)0.0;
		float yx = (float)0.0;
		float yx2 = (float)0.0;

		// Regression Data Accumulation	
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
            float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float snew 	= fmaxf(s1, s2);
			float cnew  = discount * cashFlow[path];
			
			float s;
			float c;
			float plusOne;
			
			if(fmaxf(callPut*(snew-K),(float) 0.0f) > 0.0f)
			{
				s = snew;
				c = cnew;
				plusOne = (float) 1.0;
			}
			else
			{
				s = (float) 0.0;
				c = (float) 0.0;
				plusOne = (float) 0.0;				
			}
            
			x0 	+= plusOne;
                
			x1  += s;
			x2  += s*s;
			x3  += s*s*s; 
			x4  += s*s*s*s;
			
			y   += c;
			yx  += c*s;
			yx2 += c*s*s;
	    }	
	    
		
		// Cholesky Decomposition
        // ----------------------
		//% one
		float l11 = sqrtf(x0);
		//%two
		float l21 = x1/l11;
		float l31 = x2/l11;
		//%three
		float l22 = sqrtf(x2-l21*l21);
		float l32 = (x3-l21*l31)/l22;
		//%four
		float l33 = sqrtf((x4-l31*l31)-l32*l32);
		//% Forward substitution
		float z1  = y /l11;
		float z2  = (yx-l21*z1)/l22;
		float z3  = (yx2-l31*z1-l32*z2)/l33;
		//% Backward substitution
		float b2  = z3 / l33;
		float b1  = (z2 - l32*b2) / l22;
		float b0  = (z1 - l21*b1 - l31*b2)/l11;            
        
        // display
        //printf("@ step = %d, (x0) (b0, b1, b2) = (%.1f) (%.5f, %.5f, %.5f)\n",step,x0,b0,b1,b2);
               
	    // Update cashflow
		stockIndex = step * pathsMC;

		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float s 	= fmaxf(s1, s2);
			float c		= discount * cashFlow[path];

			float payoff = fmaxf(callPut*(s-K), (float)0.0f);
            
            float priceHold = b0 + b1*s + b2*s*s;
          
            float newCash;
            
            if( (payoff > (float) 0.0f) && (payoff > priceHold) )
                newCash = payoff;
            else
                newCash = c;

            cashFlow[path] = newCash;
	    }
	}
	
	// ----------------------------------------------------------
	// average
	float average = (float) 0.0;
	
	for (uint32_t path = 0; path < pathsMC; ++path)
	    average += cashFlow[path];
	    
	float optionValue = average / pathsMC;
		
	free(normal1);
	free(normal2);
	free(stock1);
	free(stock2);
	free(lastS1);
	free(lastS2);
	free(cashFlow);
	
	return optionValue;
}
 
 
 
 
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
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
						const uint32_t seedValue2 )
{
	
	int normalIndex;
	int stockIndex;
/*
	float normal1[MAXPATHS][MAXSTEPS];
	float normal2[MAXPATHS][MAXSTEPS];
	float stock1[MAXPATHS][MAXSTEPS+1];
	float stock2[MAXPATHS][MAXSTEPS+1];
	float lastS1[MAXPATHS];
	float lastS2[MAXPATHS];
	float cashFlow[MAXPATHS];
*/
	// MEMORY ALLOCATION

	float *normal1 	= (float*) malloc( (stepsMC  ) * pathsMC * sizeof(float) );
	float *normal2 	= (float*) malloc( (stepsMC  ) * pathsMC * sizeof(float) );
	float *stock1 	= (float*) malloc( (stepsMC+1) * pathsMC * sizeof(float) );
	float *stock2 	= (float*) malloc( (stepsMC+1) * pathsMC * sizeof(float) );
	float *lastS1 	= (float*) malloc( pathsMC * sizeof(float) );
	float *lastS2 	= (float*) malloc( pathsMC * sizeof(float) );
	float *cashFlow = (float*) malloc( pathsMC * sizeof(float) );

	if( normal1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc normal1\n");
		return -1;
	}
	if( normal2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc normal2\n");
		return -1;
	}
	if( stock1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc stock1\n");
		return -1;
	}
	if( stock2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc stock2\n");
		return -1;
	}
    if( lastS1 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc lastS1\n");
		return -1;
	}
	if( lastS2 == NULL )
	{
		printf("ERROR: ls_golden unable to malloc lastS2\n");
		return -1;
	}
	if( cashFlow == NULL )
	{
		printf("ERROR: ls_golden unable to malloc cashFlow\n");
		return -1;
	}

	// ----------------------------------------------------------
	// Check callPut value
	if( (callPut != 1) && (callPut != -1) )
	{
		printf("ERROR: ls_golden: callPut value can only be 1 (call) or -1 (put)\n");
		return -1;
	}

	// ----------------------------------------------------------
	// GENERATE RANDOM NUMBERS via Mersenne-Twister and BoxMuller
	// normal 1
	init_genrand(seedValue1);
	generate_numbers();

	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal1[normalIndex] = z0;
			++normalIndex;
			normal1[normalIndex] = z1;
			++normalIndex;
		}
	}

	// normal 2
	init_genrand(seedValue2);
	generate_numbers();

	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal2[normalIndex] = z0;
			++normalIndex;
			normal2[normalIndex] = z1;
			++normalIndex;
		}
	}

	// ----------------------------------------------------------
	// GENERATE PATHS via Black-Scholes
	normalIndex = 0;
	stockIndex  = 0;

	for (uint32_t step = 0; step <= stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1;
			float s2;

			// spot price
			if(step == 0)
			{
				s1 = S0_1;
				s2 = S0_2;
			}
			else
			{
				// read normal numbers and correlate them with coefficient rho
				float n1 = normal1[normalIndex];
				float n2 = normal2[normalIndex];

				float z1 = n1;
				float z2 = rho*z1 + sqrt_1_minus_rho2 * n2;

				// Black-Scholes
				float prevS1 = lastS1[path];
				float prevS2 = lastS2[path];

				s1  = prevS1 * expf(m11 + m12 * z1);
				s2  = prevS2 * expf(m21 + m22 * z2);

				++normalIndex;
			}

			lastS1[path] = s1;
			lastS2[path] = s2;

			stock1[stockIndex] = s1;
			stock2[stockIndex] = s2;

			++stockIndex;
		}
	}
	
	// ----------------------------------------------------------
    // CASH FLOW @ Maturity
	stockIndex = stepsMC;
	for (uint32_t path = 0; path < pathsMC; ++path)
	{
		float s1 = stock1[stockIndex];
		float s2 = stock2[stockIndex];
		float Smax = fmaxf(s1, s2);
		cashFlow[path] = fmaxf(callPut*(Smax-K), (float) 0.0f);
		++stockIndex;
	}
	
	// ----------------------------------------------------------
	float accum_x0[16];
	float accum_x1[16];
	float accum_x2[16];
	float accum_x3[16];
	float accum_x4[16];
	float accum_y [16];
	float accum_yx[16];
	float accum_yx2[16];

	// LONGSTAFF-SCHWARTZ
    for (int32_t step = (stepsMC-1); step >=0; --step)
	{
		stockIndex = step * pathsMC;
		uint8_t accumIndex = 0;

		float x0 = (float)0.0;
		float x1 = (float)0.0;
		float x2 = (float)0.0;
		float x3 = (float)0.0;
		float x4 = (float)0.0;
		float y  = (float)0.0;
		float yx = (float)0.0;
		float yx2 = (float)0.0;

		for(int i=0; i<16; ++i)
		{
			accum_x0[i] = 0.0f;
			accum_x1[i] = 0.0f;
			accum_x2[i] = 0.0f;
			accum_x3[i] = 0.0f;
			accum_x4[i] = 0.0f;
			accum_y [i] = 0.0f;
			accum_yx[i] = 0.0f;
			accum_yx2[i] = 0.0f;
		}

		// Regression Data Accumulation
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
            float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float snew 	= fmaxf(s1, s2);
			float cnew  = discount * cashFlow[path];

			float s;
			float c;
			float plusOne;

			if(fmaxf(callPut*(snew-K),(float) 0.0f) > 0.0f)
			{
				s = snew;
				c = cnew;
				plusOne = (float) 1.0;
			}
			else
			{
				s = (float) 0.0;
				c = (float) 0.0;
				plusOne = (float) 0.0;
			}

			accum_x0[accumIndex]	+= plusOne;

			accum_x1[accumIndex]  	+= s;
			accum_x2[accumIndex]  	+= s*s;
			accum_x3[accumIndex]  	+= s*s*s;
			accum_x4[accumIndex]  	+= s*s*s*s;

			accum_y [accumIndex]  	+= c;
			accum_yx[accumIndex]  	+= c*s;
			accum_yx2 [accumIndex]	+= c*s*s;

			++accumIndex;
			if(accumIndex >= 16)
				accumIndex = 0;
	    }

		for(int i=0; i<16; ++i)
		{
			x0	+= accum_x0[i];
			x1	+= accum_x1[i];
			x2  += accum_x2[i];
			x3 	+= accum_x3[i];
			x4	+= accum_x4[i];
			y 	+= accum_y [i];
			yx	+= accum_yx[i];
			yx2 += accum_yx2[i];
		}

		// Cholesky Decomposition
        // ----------------------
		//% one
		float l11 = sqrtf(x0);
		//%two
		float l21 = x1/l11;
		float l31 = x2/l11;
		//%three
		float l22 = sqrtf(x2-l21*l21);
		float l32 = (x3-l21*l31)/l22;
		//%four
		float l33 = sqrtf((x4-l31*l31)-l32*l32);
		//% Forward substitution
		float z1  = y /l11;
		float z2  = (yx-l21*z1)/l22;
		float z3  = (yx2-l31*z1-l32*z2)/l33;
		//% Backward substitution
		float b2  = z3 / l33;
		float b1  = (z2 - l32*b2) / l22;
		float b0  = (z1 - l21*b1 - l31*b2)/l11;

        // display
        //printf("@ step = %d, (x0) (b0, b1, b2) = (%.1f) (%.5f, %.5f, %.5f)\n",step,x0,b0,b1,b2);


	    // Update cashflow
		stockIndex = step * pathsMC;

		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float s 	= fmaxf(s1, s2);
			float c		= discount * cashFlow[path];

			float payoff = fmaxf(callPut*(s-K), (float)0.0f);

            float priceHold = b0 + b1*s + b2*s*s;

            float newCash;

            if( (payoff > (float) 0.0f) && (payoff > priceHold) )
                newCash = payoff;
            else
                newCash = c;

            cashFlow[path] = newCash;
	    }
	}

	// ----------------------------------------------------------
	// average
	float average = (float) 0.0;

	for (uint32_t path = 0; path < pathsMC; ++path)
	    average += cashFlow[path];

	float optionValue = average / pathsMC;

	free(normal1);
	free(normal2);
	free(stock1);
	free(stock2);
	free(lastS1);
	free(lastS2);
	free(cashFlow);
	
	return optionValue;
}
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

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
					const uint32_t seedValue2 )
{
	int normalIndex;
	int stockIndex;
	
	float normal1[MAXPATHS*MAXSTEPS];
	float normal2[MAXPATHS*MAXSTEPS];
	float stock1[MAXPATHS*(MAXSTEPS+1)];
	float stock2[MAXPATHS*(MAXSTEPS+1)];
	float lastS1[MAXPATHS];
	float lastS2[MAXPATHS];
	float cashFlow[MAXPATHS];
	
	// ----------------------------------------------------------
	// Check callPut value
	if( (callPut != 1) && (callPut != -1) )
	{
		printf("ERROR: ls_golden: callPut value can only be 1 (call) or -1 (put)\n");
		return -1;
	}

	// ----------------------------------------------------------
	// GENERATE RANDOM NUMBERS via Mersenne-Twister and BoxMuller
	// normal 1
	init_genrand(seedValue1);
	generate_numbers();
	
	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal1[normalIndex] = z0;
			++normalIndex;
			normal1[normalIndex] = z1;
			++normalIndex;
		}
	}

	// normal 2
	init_genrand(seedValue2);
	generate_numbers();
	
	normalIndex = 0;
	for (uint32_t step = 0; step < stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; path+=2)
		{
			float u1 = genrand_real1();
			float u2 = genrand_real1();

			float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2*PI * u2);
			float z1 = sqrtf(-2.0 * logf(u1)) * sinf(2*PI * u2);

			normal2[normalIndex] = z0;
			++normalIndex;
			normal2[normalIndex] = z1;
			++normalIndex;
		}
	}

	// ----------------------------------------------------------
	// GENERATE PATHS via Black-Scholes
	normalIndex = 0;
	stockIndex  = 0;

	for (uint32_t step = 0; step <= stepsMC; ++step)
	{
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1;
			float s2;

			// spot price
			if(step == 0)
			{
				s1 = S0_1;
				s2 = S0_2;
			}						
			else
			{
				// read normal numbers and correlate them with coefficient rho
				float n1 = normal1[normalIndex];
				float n2 = normal2[normalIndex];

				float z1 = n1;
				float z2 = rho*z1 + sqrt_1_minus_rho2 * n2;

				// Black-Scholes
				float prevS1 = lastS1[path];
				float prevS2 = lastS2[path];

				s1  = prevS1 * expf(m11 + m12 * z1);
				s2  = prevS2 * expf(m21 + m22 * z2);
				
				++normalIndex;
			}

			lastS1[path] = s1;
			lastS2[path] = s2;
			 
			stock1[stockIndex] = s1;
			stock2[stockIndex] = s2;

			++stockIndex;
		}
	}
	
	// ----------------------------------------------------------
    // CASH FLOW @ Maturity
	stockIndex = stepsMC;
	for (uint32_t path = 0; path < pathsMC; ++path)
	{
		float s1 = stock1[stockIndex];
		float s2 = stock2[stockIndex];
		float Smax = fmaxf(s1, s2);
		cashFlow[path] = fmaxf(callPut*(Smax-K), (float) 0.0f);
		++stockIndex;
	}

	// ----------------------------------------------------------
	// LONGSTAFF-SCHWARTZ
    for (int32_t step = (stepsMC-1); step >=0; --step)
	{
		stockIndex = step * pathsMC;

		float x0 = (float)0.0;
		float x1 = (float)0.0;
		float x2 = (float)0.0;
		float x3 = (float)0.0;
		float x4 = (float)0.0;
		float y  = (float)0.0;
		float yx = (float)0.0;
		float yx2 = (float)0.0;

		// Regression Data Accumulation	
		for (uint32_t path = 0; path < pathsMC; ++path)
		{
            float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float snew 	= fmaxf(s1, s2);
			float cnew  = discount * cashFlow[path];
			
			float s;
			float c;
			float plusOne;
			
			if(fmaxf(callPut*(snew-K),(float) 0.0f) > 0.0f)
			{
				s = snew;
				c = cnew;
				plusOne = (float) 1.0;
			}
			else
			{
				s = (float) 0.0;
				c = (float) 0.0;
				plusOne = (float) 0.0;				
			}
            
			x0 	+= plusOne;
                
			x1  += s;
			x2  += s*s;
			x3  += s*s*s; 
			x4  += s*s*s*s;
			
			y   += c;
			yx  += c*s;
			yx2 += c*s*s;
	    }	
	    
		
		// Cholesky Decomposition
        // ----------------------
		//% one
		float l11 = sqrtf(x0);
		//%two
		float l21 = x1/l11;
		float l31 = x2/l11;
		//%three
		float l22 = sqrtf(x2-l21*l21);
		float l32 = (x3-l21*l31)/l22;
		//%four
		float l33 = sqrtf((x4-l31*l31)-l32*l32);
		//% Forward substitution
		float z1  = y /l11;
		float z2  = (yx-l21*z1)/l22;
		float z3  = (yx2-l31*z1-l32*z2)/l33;
		//% Backward substitution
		float b2  = z3 / l33;
		float b1  = (z2 - l32*b2) / l22;
		float b0  = (z1 - l21*b1 - l31*b2)/l11;            
        
        // display
        //printf("@ step = %d, (x0) (b0, b1, b2) = (%.1f) (%.5f, %.5f, %.5f)\n",step,x0,b0,b1,b2);
               
	    // Update cashflow
		stockIndex = step * pathsMC;

		for (uint32_t path = 0; path < pathsMC; ++path)
		{
			float s1 	= stock1[stockIndex];
			float s2 	= stock2[stockIndex];
			++stockIndex;
			float s 	= fmaxf(s1, s2);
			float c		= discount * cashFlow[path];

			float payoff = fmaxf(callPut*(s-K), (float)0.0f);
            
            float priceHold = b0 + b1*s + b2*s*s;
          
            float newCash;
            
            if( (payoff > (float) 0.0f) && (payoff > priceHold) )
                newCash = payoff;
            else
                newCash = c;

            cashFlow[path] = newCash;
	    }
	}
	
	// ----------------------------------------------------------
	// average
	float average = (float) 0.0;
	
	for (uint32_t path = 0; path < pathsMC; ++path)
	    average += cashFlow[path];
	    
	float optionValue = average / pathsMC;
	
	return optionValue;
}
 
 
 
 
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------