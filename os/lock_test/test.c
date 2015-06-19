int TEST (void)
{
	STRUCT *p;
	double start, stop;
	int i, sum;
	p = (STRUCT *)malloc(sizeof(STRUCT)*nmbrElems+1);
	if (!p) {
		printf("%s ERROR: Cannot malloc.\n", NAME);
		return 1;
	}

#pragma omp parallel for schedule(static)
	for (i=0; i < nmbrElems; ++i)
		INIT(p+i);

#pragma omp parallel
	{
		uint32_t seed;
		unsigned j;
		perm = (unsigned *)malloc((size_t)2*nmbrElems*sizeof(unsigned));
		seed = (uint32_t)omp_get_wtime();
		for (j=0; j < 2*nmbrElems; ++j)
			perm[j] = (unsigned)rand_r(&seed) % nmbrElems;
	}

	start = omp_get_wtime();
#pragma omp parallel
	{
		unsigned j;
#pragma omp barrier
		for (j=0; j < nIter; ++j) {
			ADD(p + perm[2*(j%nmbrElems) + 0],  1);
			SUB(p + perm[2*(j%nmbrElems) + 1], -1);
		}
	}
	stop = omp_get_wtime();

#pragma omp parallel
	{
		(void)free((void *)perm);
	}

	for (i=0,sum=0; i<nmbrElems; ++i)
		sum+=VALUE(p[i]);

	printf(" (%3ld)	 %-35s: %s (%d threads %12.3f ms) %12.3f ns/op\n",
	       sizeof(STRUCT), NAME, (sum == 0) ? "OK" : "ERROR", nThreads,
	       (stop-start)*1e+3, ((stop-start)*1e+9)/(nIter*2));

	free((void*)p);
	return sum != 0;
}
