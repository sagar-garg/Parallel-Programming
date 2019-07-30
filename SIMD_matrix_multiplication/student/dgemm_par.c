#include "dgemm.h"
#include <immintrin.h>
#include <inttypes.h>


/*a fancy solution from github for horizontal sum along a vector. could be used 
static inline float hsum(__m256 x) {
    const __m128 hiQuad = _mm256_extractf128_ps(x, 1);
    const __m128 loQuad = _mm256_castps256_ps128(x);
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
    const __m128 loDual = sumQuad;
    const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
    const __m128 sumDual = _mm_add_ps(loDual, hiDual);
    const __m128 lo = sumDual;
    const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    const __m128 sum = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(sum);
}
  */

 //Scalar function to sum horizontally along a vector. not enough speedup
 inline float mm256_sum1(__m256 reg) {
  float TmpRes[8];
  _mm256_store_ps(TmpRes, reg);
  return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3] +
         TmpRes[4] + TmpRes[5] + TmpRes[6] + TmpRes[7];
}


//vector function to sum horizontally along a vector.  enough speedup
 inline float mm256_sum2(__m256 reg) {
  __m128 sum = _mm_add_ps(_mm256_extractf128_ps(reg, 0),
                          _mm256_extractf128_ps(reg, 1));
  
  float TmpRes[4];
  _mm_store_ps(TmpRes, sum);
return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3];
}

void dgemm(float *a, float *b, float *c, int n) {
    __m256 a_k, b_k, mul;
    int32_t mask_src[] = {-1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0}; //for remainders
    

    //processes all except remainder not divisible by 8
    int n1=n-(n%8);
    int rem=n%8;
    //float tmp[8];

    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            for(int k = 0; k < n1; k+=8){
                //c[i * n + j] += a[i * n  + k] * b[j * n  + k];
                    a_k = _mm256_loadu_ps(a+i*n+k);
                    b_k = _mm256_loadu_ps(b+j*n+k);
                    mul = _mm256_mul_ps(a_k, b_k);

                //horizontal sum
                c[i*n+j] += mm256_sum2(mul);
            }
        }
    }

//process remainder
for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            for(int k = n1; k < n; k+=8){
                //c[i * n + j] += a[i * n  + k] * b[j * n  + k];
                __m256i imask = _mm256_loadu_si256((__m256i const *)(mask_src + 8 - rem));
                a_k = _mm256_maskload_ps(a+i*n+k, imask);
                    b_k = _mm256_maskload_ps(b+j*n+k, imask);
                 mul = _mm256_mul_ps(a_k, b_k);
                              //horizontal sum
                c[i*n+j] += mm256_sum2(mul);
  
                } 
            }
        }
    }
