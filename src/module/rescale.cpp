/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 21:14:57
 * @modify date 2023-09-01 21:14:57
 * @desc [description]
 */

#include "rescale.h"
#include "logging.h"
#include "dedisperse.h"

Rescale::Rescale()
{
}

Rescale::~Rescale()
{
}

void Rescale::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	resize(nsamples, nchans);

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	weights.resize(nchans, 1.);

	chmean.resize(nchans, 0.);
	chstd.resize(nchans, 0.);
	chweight.resize(nchans, 0.);
}

DataBuffer<float> * Rescale::filter(DataBuffer<float> &databuffer)
{
	if (chweight.empty())
	{
		BOOST_LOG_TRIVIAL(warning)<<"no mean and stddev found!";
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform rescale";

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for (long int i=0; i<databuffer.nsamples; i++)
	{
		for (long int j=0; j<databuffer.nchans; j++)
		{
			databuffer.buffer[i*databuffer.nchans+j] = chweight[j]*(databuffer.buffer[i*databuffer.nchans+j]-chmean[j])/chstd[j];
		}
	}

	counter += nsamples;
	databuffer.isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return databuffer.get();
}