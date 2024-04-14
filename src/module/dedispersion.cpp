/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 22:43:06
 * @modify date 2023-09-01 22:43:06
 * @desc [description]
 */

#include "dedispersion.h"
#include "logging.h"
#include "dedisperse.h"
#include "utils.h"
#include <algorithm>

Dedispersion::Dedispersion()
{
	dm = 0.;
	buf_size = 0;
	offset = 0;
}

Dedispersion::~Dedispersion()
{
}

void Dedispersion::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	weights.resize(nchans, 1.);

	delayn.resize(nchans, 0);

	double fmax = *std::max_element(frequencies.begin(), frequencies.end());
	double fmin = *std::min_element(frequencies.begin(), frequencies.end());

	int maxdelayn = std::ceil(dmdelay(dm, fmax, fmin) / tsamp);
	maxdelayn = (int) std::ceil(maxdelayn * 1. / nsamples) * nsamples;

	buf_size = nsamples + maxdelayn;
	buf.resize(buf_size * nchans);

	buffer.resize(nsamples * nchans, 0.);

	size_t nchans_real = frequencies.size();
	size_t nifs = nchans / nchans_real;
	for (size_t k=0; k<nifs; k++)
	{
		for (size_t j=0; j<nchans_real; j++)
		{
			delayn[k * nchans_real + j] = std::round(dmdelay(dm, fmax, frequencies[j]) / tsamp);
		}
	}

	offset = buf_size - nsamples;
}

DataBuffer<float> * Dedispersion::run(DataBuffer<float> &databuffer)
{
	if (dm == 0.) return databuffer.get();

	int nspace = buf_size - nsamples;

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			buf[(i+nspace)*nchans+j] = databuffer.buffer[i*nchans+j];
		}
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform dedispersion";

	databuffer.isbusy = false;
	if (databuffer.closable) databuffer.close();

	if (nchans % 16 == 0 && nsamples % 16 == 0)
	{
		for (long int i=0; i<nsamples/16; i++)
		{
			for (long int j=0; j<nchans/16; j++)
			{
#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
				for (long int n=0; n<16; n++)
				{
					for (long int m=0; m<16; m++)
					{
						buffer[(i * 16 + n) * nchans + (j * 16 + m)] = buf[(i * 16 + n + delayn[j * 16 + m]) * nchans + (j * 16 + m)];
					}
				}
			}
		}
	}
	else
	{
		for (long int i=0; i<nsamples; i++)
		{
			for (long int j=0; j<nchans; j++)
			{
				buffer[i * nchans + j] = buf[(i + delayn[j]) * nchans + j];
			}
		}
	}

	for (long int i=0; i<nspace; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			buf[i*nchans+j] = buf[(i+nsamples)*nchans+j];
		}
	}

	counter += nsamples;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}