/*********************************************************************/
/* Copyright 2009, 2010 The University of Texas at Austin.           */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include <stdio.h>
#include "common.h"
#ifdef FUNCTION_PROFILE
#include "functable.h"
#endif

#ifdef SMP
static int asum_threads (BLASLONG m, BLASLONG n, BLASLONG k, float alpha,
	       float* x, BLASLONG incx, float* y, BLASLONG incy, float* z, BLASLONG incz)
{
#ifndef CBLAS
  FLOATRET ret;
  ret = (FLOATRET)ASUM_K(m, x, incx);
  *((double *)z) = (double)ret;
#else
  FLOAT ret;
  ret = ASUM_K(m, x, incx);
  *((double *)z) = (double)ret;
#endif

  return 0;
}
#endif

#ifndef CBLAS

FLOATRET NAME(blasint *N, FLOAT *x, blasint *INCX){

  BLASLONG n    = *N;
  BLASLONG incx = *INCX;
  FLOATRET ret;

  PRINT_DEBUG_NAME;

#ifdef SMP
  int i;
  int mode, nthreads;
  double mid_result= 0.0;
  FLOAT dummyalpha[2] = {ZERO, ZERO};
  double *buffer = (double*)blas_memory_alloc(0);
#endif

  if (n <= 0) return 0;

  IDEBUG_START;

  FUNCTION_PROFILE_START();

#ifdef SMP
  nthreads = num_cpu_avail(1);

  //Temporarily work-around the low performance issue with small imput size &
  //multithreads.
  if (n <= 100000)
	  nthreads = 1;

  if (nthreads == 1) {
#endif

  ret = (FLOATRET)ASUM_K(n, x, incx);

#ifdef SMP
  } else {

#ifndef DOUBLE 
#ifndef COMPLEX
    mode  =  BLAS_SINGLE | BLAS_REAL;
#else
    mode  =  BLAS_SINGLE | BLAS_COMPLEX;
#endif
#else
#ifndef COMPLEX
    mode  =  BLAS_DOUBLE | BLAS_REAL;
#else
    mode  =  BLAS_DOUBLE | BLAS_COMPLEX;
#endif
#endif

    blas_level1_thread_with_return_value(mode, n, 0, 0, dummyalpha,
		       x, incx, NULL, 0, buffer, 0, (void *)asum_threads, nthreads);

    for(i = 0; i < nthreads; i++)
        mid_result += buffer[2*i];
    
    ret = (FLOATRET)mid_result;
  }

 blas_memory_free(buffer);
#endif

  FUNCTION_PROFILE_END(COMPSIZE, n, n);

  IDEBUG_END;

  return ret;
}

#else

FLOAT CNAME(blasint n, FLOAT *x, blasint incx){

  FLOAT ret;

  PRINT_DEBUG_CNAME;

#ifdef SMP
  int i;
  int mode, nthreads;
  double mid_result= 0.0;
  FLOAT dummyalpha[2] = {ZERO, ZERO};

  double *buffer = (double*)blas_memory_alloc(0);
#endif

  if (n <= 0) return 0;

  IDEBUG_START;

  FUNCTION_PROFILE_START();

#ifdef SMP
  nthreads = num_cpu_avail(1);

  //Temporarily work-around the low performance issue with small imput size &
  //multithreads.
  if (n <= 100000)
	  nthreads = 1;

  if (nthreads == 1) {
#endif

  ret = ASUM_K(n, x, incx);

#ifdef SMP
  } else {

#ifndef DOUBLE 
#ifndef COMPLEX
    mode  =  BLAS_SINGLE | BLAS_REAL;
#else
    mode  =  BLAS_SINGLE | BLAS_COMPLEX;
#endif
#else
#ifndef COMPLEX
    mode  =  BLAS_DOUBLE | BLAS_REAL;
#else
    mode  =  BLAS_DOUBLE | BLAS_COMPLEX;
#endif
#endif

    blas_level1_thread_with_return_value(mode, n, 0, 0, dummyalpha,
		       x, incx, NULL, 0, buffer, 0, (void *)asum_threads, nthreads);

    for(i = 0; i < nthreads; i++)
        mid_result += buffer[2*i];
    
    ret = (FLOAT)mid_result;
  }

 blas_memory_free(buffer);
#endif

  FUNCTION_PROFILE_END(COMPSIZE, n, n);

  IDEBUG_END;


  return ret;
}

#endif
