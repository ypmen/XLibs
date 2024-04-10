/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-10 20:34:03
 * @modify date 2024-04-10 20:34:03
 * @desc [description]
 */

#include "stat2.h"
#include "utils.h"

Stat2::Stat2(){}

Stat2::~Stat2(){}

void Stat2::prepare(DataBuffer<unsigned char> &databuffer)
{
	frequencies = databuffer.frequencies;

	hists.resize(frequencies.size() * 256);
}

void Stat2::run(DataBuffer<unsigned char> &databuffer)
{
	int nchans = databuffer.nchans;
	int nsamples = databuffer.nsamples;

	std::vector<int> histsT(256 * frequencies.size(), 0);

	for (size_t i=0; i<nsamples; i++)
	{
		for (size_t j=0; j<nchans; j++)
		{
			unsigned char d = databuffer.buffer[i * nchans + j];
			histsT[d * nchans + j] += 1;
		}
	}

	transpose_pad<int>(hists.data(), histsT.data(), 256, nchans);
}
