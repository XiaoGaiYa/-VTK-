#pragma once


struct FiltersSets
{
	float sigma_gaussion;
	float sigma_bilateral;
	float threshold;
	int kernelsize_gaussion;
	int kernelsize_mid;
	int iterations;
};

struct ThresholdSets
{
	int low;
	int high;
	bool is_inside;
};
