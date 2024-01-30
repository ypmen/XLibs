/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 21:29:33
 * @modify date 2023-09-01 21:29:33
 * @desc [description]
 */

#include "defaraday.h"
#include "logging.h"
#include "constants.h"
#include "dedisperse.h"

Defaraday::Defaraday()
{
	rm = 0;
	nifs = 1;
}

Defaraday::~Defaraday()
{
}

void Defaraday::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	resize(nsamples, nchans);

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	weights.resize(nchans, 1.);

	nchans_real = frequencies.size();
	nifs = nchans / nchans_real;

	cos_2psi.resize(nchans_real, 1.);
	sin_2psi.resize(nchans_real, 0.);

	for (size_t j=0; j<nchans_real; j++)
	{
		double psi = rm / (frequencies[j] * frequencies[j]) * 1e-12 * CONST_C * CONST_C;
		cos_2psi[j] = std::cos(-2. * psi);
		sin_2psi[j] = std::sin(-2. * psi);
	}
}

DataBuffer<float> * Defaraday::filter(DataBuffer<float> &databuffer)
{
	if (nifs != 4 or rm == 0.)
	{
		BOOST_LOG_TRIVIAL(warning)<<"no RM correction";
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform defaraday";

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for (size_t i=0; i<databuffer.nsamples; i++)
	{
		for (size_t j=0; j<nchans_real; j++)
		{
			float xx = databuffer.buffer[i * 4 * nchans_real + 0 * nchans_real + j];
			float yy = databuffer.buffer[i * 4 * nchans_real + 1 * nchans_real + j];
			float xy_re = databuffer.buffer[i * 4 * nchans_real + 2 * nchans_real + j];
			float xy_im = databuffer.buffer[i * 4 * nchans_real + 3 * nchans_real + j];

			float I = xx + yy;
			float Q_tmp = xx - yy;
			float U_tmp = 2. * xy_re;
			float V = 2. * xy_im;

			float Q = Q_tmp * cos_2psi[j] - U_tmp * sin_2psi[j];
			float U = Q_tmp * sin_2psi[j] + U_tmp * cos_2psi[j];

			databuffer.buffer[i * 4 * nchans_real + 0 * nchans_real + j] = I;
			databuffer.buffer[i * 4 * nchans_real + 1 * nchans_real + j] = Q;
			databuffer.buffer[i * 4 * nchans_real + 2 * nchans_real + j] = U;
			databuffer.buffer[i * 4 * nchans_real + 3 * nchans_real + j] = V;
		}
	}

	counter += nsamples;
	databuffer.isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return databuffer.get();
}

DataBuffer<float> * Defaraday::run(DataBuffer<float> &databuffer)
{
	if (nifs != 4 or rm == 0.)
	{
		BOOST_LOG_TRIVIAL(warning)<<"no RM correction";
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform defaraday";

	int nchans_real = databuffer.nchans / 4;

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for (size_t i=0; i<databuffer.nsamples; i++)
	{
		for (size_t j=0; j<nchans_real; j++)
		{
			float xx = databuffer.buffer[i * 4 * nchans_real + 0 * nchans_real + j];
			float yy = databuffer.buffer[i * 4 * nchans_real + 1 * nchans_real + j];
			float xy_re = databuffer.buffer[i * 4 * nchans_real + 2 * nchans_real + j];
			float xy_im = databuffer.buffer[i * 4 * nchans_real + 3 * nchans_real + j];

			float I = xx + yy;
			float Q_tmp = xx - yy;
			float U_tmp = 2. * xy_re;
			float V = 2. * xy_im;

			float Q = Q_tmp * cos_2psi[j] - U_tmp * sin_2psi[j];
			float U = Q_tmp * sin_2psi[j] + U_tmp * cos_2psi[j];

			buffer[i * 4 * nchans_real + 0 * nchans_real + j] = I;
			buffer[i * 4 * nchans_real + 1 * nchans_real + j] = Q;
			buffer[i * 4 * nchans_real + 2 * nchans_real + j] = U;
			buffer[i * 4 * nchans_real + 3 * nchans_real + j] = V;
		}
	}

	counter += nsamples;
	isbusy = true;

	databuffer.isbusy = false;
	if (databuffer.closable) databuffer.close();

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}