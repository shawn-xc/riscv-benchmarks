#include "mt_histo.h"
#include "dataset1.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

#define NTHREADS 16

struct histo_data {
	int start;
	int end;
	int nbins;
	int *input_data;
	int *output_bins;
};

void histo_kernel(int *in_data, int ndata, int *out_bins, int nbins)
{
	int i, oldsum;
	int localbins[nbins];

	memset(localbins, 0, sizeof(int) * nbins);

	for (i = 0; i < ndata; i++) {
		int pos = in_data[i];
		localbins[pos]++;
	}

	for (i = 0; i < nbins; i++) {
		int *dst;

		if (localbins[i] == 0)
			continue;

		dst = &out_bins[i];
		asm volatile ("amoadd.w %[oldsum], %[count], (%[dst])" :
				[oldsum] "=r" (oldsum) :
				[count] "r" (localbins[i]),
				[dst] "r" (dst));
	}
}

void printBins(int *out_bins, int nbins)
{
	int i;

	for (i = 0; i < nbins; i++)
		printf("out_bins[%d] = %d\n", i, out_bins[i]);
}

#define min(a, b) ((a) < (b)) ? (a) : (b)


void thread_entry(int cid, int nc)
{
	int local_data_size = (DATA_SIZE - 1) / nc + 1;
	int start = cid * local_data_size;
	int end = min(start + local_data_size, DATA_SIZE);

	barrier(nc);

	if (cid == 0)
		setStats(1);

	histo_kernel(input_data + start, end - start, output_bins, NUM_BINS);

	barrier(nc);

	if (cid == 0) {
		setStats(0);

		// Print out the result
		printArray("results", NUM_BINS, output_bins);

		// Check the results
		exit(verify(NUM_BINS, output_bins, verify_data));
	} else {
		exit(0);
	}
}
