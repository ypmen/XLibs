/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-07-09 20:21:06
 * @modify date 2024-07-09 20:21:06
 * @desc [description]
 */

#include <string.h>

#include "dedisperse.h"
#include "flip.h"
#include "logging.h"

using namespace std;

Flip::Flip(){}

Flip::~Flip(){}

void Flip::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;
	
	tsamp = databuffer.tsamp;

	std::reverse(databuffer.frequencies.begin(), databuffer.frequencies.end());

	frequencies = databuffer.frequencies;
}

DataBuffer<float> * Flip::filter(DataBuffer<float> &databuffer)
{
	BOOST_LOG_TRIVIAL(debug)<<"flip the frequency";

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for (long int i=0; i<nsamples; i++)
	{
		std::reverse(databuffer.buffer.begin()+i*nchans, databuffer.buffer.begin()+(i+1)*nchans);
	}
	
	databuffer.equalized = false;
	counter += nsamples;

	databuffer.isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return databuffer.get();
}
