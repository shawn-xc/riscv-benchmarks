// See LICENSE for license details.

#include "stdlib.h"
#include "dataset.h"

//--------------------------------------------------------------------------
// vvadd function

void __attribute__((noinline)) vvadd(int coreid, int ncores, size_t n, const data_t* x, const data_t* y, data_t* z)
{
   size_t i;
   size_t chunk_sz = n / ncores;
   size_t base = coreid * chunk_sz;
   size_t max = base + chunk_sz > n ? n : base + chunk_sz;

   for (i = coreid * chunk_sz; i < max; i++)
   {
      z[i] = x[i] + y[i];
   }
}
