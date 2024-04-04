/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2021-01-17 14:26:45
 * @modify date 2021-01-17 14:26:45
 * @desc [description]
 */

#ifdef __AVX2__
#include "avx2.h"
#endif

#include "baseline.h"
#include "dedisperse.h"
#include "utils.h"
#include "logging.h"

BaseLine::BaseLine()
{
	width = 0.;
}

BaseLine::BaseLine(nlohmann::json &config)
{
	width = config["width"];
}

BaseLine::~BaseLine(){}

void BaseLine::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	resize(nsamples, nchans);

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	weights.resize(nchans, 0.);

	if (int(width/tsamp) >= 3)
	{
		std::vector<std::pair<std::string, std::string>> meta = {
			{"nsamples", std::to_string(nsamples)},
			{"nchans", std::to_string(nchans)},
			{"tsamp", std::to_string(tsamp)},
			{"width", std::to_string(width)}
		};
		format_logging("Baseline Removal Info", meta);
	}
}

DataBuffer<float> * BaseLine::filter(DataBuffer<float> &databuffer)
{
	if (int(width/tsamp) < 3)
	{
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform baseline removal with time scale="<<width;

#ifndef __AVX2__
	vector<double> xe(nchans, 0.);
	vector<double> xs(nchans, 0.);
	vector<double> alpha(nchans, 0.);
	vector<double> beta(nchans, 0.);
	vector<double> szero(nsamples, 0.);
	vector<double> s(nsamples, 0.);
	double se = 0.;
	double ss = 0.;

	if (outref.empty())
	{
		for (long int i=0; i<nsamples; i++)
		{
			double temp = 0.;
			for (long int j=0; j<nchans; j++)
			{
				temp += databuffer.buffer[i*nchans+j];
			}
			szero[i] = temp/nchans;
		}

		runMedian2(szero.data(), s.data(), nsamples, width/tsamp);
	}
	else
	{
		std::copy(outref.begin()+counter, outref.begin()+counter+nsamples, s.begin());
	}

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			xe[j] += databuffer.buffer[i*nchans+j];
		}

		se += s[i];
		ss += s[i]*s[i];
		
		for (long int j=0; j<nchans; j++)
		{
			xs[j] += databuffer.buffer[i*nchans+j]*s[i];
		}
	}

	double tmp = se*se-ss*nsamples;
	if (tmp != 0)
	{
		for (long int j=0; j<nchans; j++)
		{
			alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
			beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
		}
	}

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			databuffer.buffer[i*nchans+j] = databuffer.buffer[i*nchans+j]-alpha[j]*s[i]-beta[j];
		}
	}

#else
	if (nchans % 8 == 0)
	{
		vector<double, boost::alignment::aligned_allocator<double, 32>> xe(nchans, 0.);
		vector<double, boost::alignment::aligned_allocator<double, 32>> xs(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> alpha(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> beta(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> szero(nsamples, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> s(nsamples, 0.);
		double se = 0.;
		double ss = 0.;

		if (outref.empty())
		{
			for (long int i=0; i<nsamples; i++)
			{
				double temp = PulsarX::reduce(databuffer.buffer.data()+i*nchans, nchans);
				szero[i] = temp/nchans;
			}

			runMedian2(szero.data(), s.data(), nsamples, width/tsamp);
		}
		else
		{
			std::copy(outref.begin()+counter, outref.begin()+counter+nsamples, s.begin());
		}

		for (long int i=0; i<nsamples; i++)
		{
			PulsarX::accumulate_mean(xe.data(), xs.data(), s[i], databuffer.buffer.data()+i*nchans, nchans);

			se += s[i];
			ss += s[i]*s[i];
		}

		double tmp = se*se-ss*nsamples;
		if (tmp != 0)
		{
			for (long int j=0; j<nchans; j++)
			{
				alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
				beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
			}
		}

		for (long int i=0; i<nsamples; i++)
		{
			PulsarX::remove_baseline(databuffer.buffer.data()+i*nchans, databuffer.buffer.data()+i*nchans, alpha.data(), beta.data(), s[i], nchans);
		}

	}
	else
	{
		vector<double> xe(nchans, 0.);
		vector<double> xs(nchans, 0.);
		vector<double> alpha(nchans, 0.);
		vector<double> beta(nchans, 0.);
		vector<double> szero(nsamples, 0.);
		vector<double> s(nsamples, 0.);
		double se = 0.;
		double ss = 0.;

		if (outref.empty())
		{
			for (long int i=0; i<nsamples; i++)
			{
				double temp = 0.;
				for (long int j=0; j<nchans; j++)
				{
					temp += databuffer.buffer[i*nchans+j];
				}
				szero[i] = temp/nchans;
			}

			runMedian2(szero.data(), s.data(), nsamples, width/tsamp);
		}
		else
		{
			std::copy(outref.begin()+counter, outref.begin()+counter+nsamples, s.begin());
		}

		for (long int i=0; i<nsamples; i++)
		{
			for (long int j=0; j<nchans; j++)
			{
				xe[j] += databuffer.buffer[i*nchans+j];
			}

			se += s[i];
			ss += s[i]*s[i];
			
			for (long int j=0; j<nchans; j++)
			{
				xs[j] += databuffer.buffer[i*nchans+j]*s[i];
			}
		}

		double tmp = se*se-ss*nsamples;
		if (tmp != 0)
		{
			for (long int j=0; j<nchans; j++)
			{
				alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
				beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
			}
		}

		for (long int i=0; i<nsamples; i++)
		{
			for (long int j=0; j<nchans; j++)
			{
				databuffer.buffer[i*nchans+j] = databuffer.buffer[i*nchans+j]-alpha[j]*s[i]-beta[j];
			}
		}
	}
#endif

	std::fill(databuffer.means.begin(), databuffer.means.end(), 0.);

	counter += nsamples;

	databuffer.isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return databuffer.get();
}

DataBuffer<float> * BaseLine::run(DataBuffer<float> &databuffer)
{
	if (int(width/tsamp) < 3)
	{
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform baseline removal with time scale="<<width;

	if (closable) open();

	vector<double> xe(nchans, 0.);
	vector<double> xs(nchans, 0.);
	vector<double> alpha(nchans, 0.);
	vector<double> beta(nchans, 0.);
	vector<double> szero(nsamples, 0.);
	vector<double> s(nsamples, 0.);
	double se = 0.;
	double ss = 0.;

	if (outref.empty())
	{
		for (long int i=0; i<nsamples; i++)
		{
			double temp = 0.;
			for (long int j=0; j<nchans; j++)
			{
				temp += databuffer.buffer[i*nchans+j];
			}
			szero[i] = temp/nchans;
		}

		runMedian2(szero.data(), s.data(), nsamples, width/tsamp);
	}
	else
	{
		std::copy(outref.begin()+counter, outref.begin()+counter+nsamples, s.begin());
	}

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			xe[j] += databuffer.buffer[i*nchans+j];
		}

		se += s[i];
		ss += s[i]*s[i];
		
		for (long int j=0; j<nchans; j++)
		{
			xs[j] += databuffer.buffer[i*nchans+j]*s[i];
		}
	}

	double tmp = se*se-ss*nsamples;
	if (tmp != 0)
	{
		for (long int j=0; j<nchans; j++)
		{
			alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
			beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
		}
	}

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			buffer[i*nchans+j] = databuffer.buffer[i*nchans+j]-alpha[j]*s[i]-beta[j];
		}
	}

	equalized = databuffer.equalized;
	std::fill(means.begin(), means.end(), 0.);
	vars = databuffer.vars;
	mean_var_ready = databuffer.mean_var_ready;
	weights = databuffer.weights;

	counter += nsamples;

	databuffer.isbusy = false;
	isbusy = true;

	if (databuffer.closable) databuffer.close();

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}

DataBuffer<float> * BaseLine::filter2(DataBuffer<float> &databuffer)
{
	if (int(width/tsamp) < 3)
	{
		return databuffer.get();
	}

	BOOST_LOG_TRIVIAL(debug)<<"perform baseline removal with time scale="<<width;

#ifndef __AVX2__
	vector<double> xe(nchans, 0.);
	vector<double> xs(nchans, 0.);
	vector<double> alpha(nchans, 0.);
	vector<double> beta(nchans, 0.);
	vector<double> szero(nsamples, 0.);
	vector<double> sstdzero(nsamples, 0.);
	vector<double> s(nsamples, 0.);
	vector<double> sstd(nsamples, 0.);
	double se = 0.;
	double ss = 0.;

	for (long int i=0; i<nsamples; i++)
	{
		double temp = 0.;
		for (long int j=0; j<nchans; j++)
		{
			temp += databuffer.buffer[i*nchans+j];
		}
		szero[i] = temp/nchans;
	}

	runMedian2(szero.data(), s.data(), nsamples, width/tsamp);

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			xe[j] += databuffer.buffer[i*nchans+j];
		}

		se += s[i];
		ss += s[i]*s[i];
		
		for (long int j=0; j<nchans; j++)
		{
			xs[j] += databuffer.buffer[i*nchans+j]*s[i];
		}
	}

	double tmp = se*se-ss*nsamples;
	if (tmp != 0)
	{
		for (long int j=0; j<nchans; j++)
		{
			alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
			beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
		}
	}

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			databuffer.buffer[i*nchans+j] = (databuffer.buffer[i*nchans+j]-alpha[j]*s[i]-beta[j]);
			sstdzero[i] +=  databuffer.buffer[i*nchans+j] * databuffer.buffer[i*nchans+j];
		}
		sstdzero[i] /= nchans;
	}

	runMedian2(sstdzero.data(), sstd.data(), nsamples, width/tsamp);

	for (long int i=0; i<nsamples; i++)
	{
		double norm = sstd[i] == 0. ? 0. : 1./sstd[i];
		for (long int j=0; j<nchans; j++)
		{
			databuffer.buffer[i*nchans+j] *= norm;
		}
	}

#else
	if (nchans % 8 == 0)
	{
		vector<double, boost::alignment::aligned_allocator<double, 32>> xe(nchans, 0.);
		vector<double, boost::alignment::aligned_allocator<double, 32>> xs(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> alpha(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> beta(nchans, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> szero(nsamples, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> sstdzero(nsamples, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> s(nsamples, 0.);
		vector<float, boost::alignment::aligned_allocator<float, 32>> sstd(nsamples, 0.);
		double se = 0.;
		double ss = 0.;

		for (long int i=0; i<nsamples; i++)
		{
			double temp = PulsarX::reduce(databuffer.buffer.data()+i*nchans, nchans);
			szero[i] = temp/nchans;
		}

		runMedian2(szero.data(), s.data(), nsamples, width/tsamp);

		for (long int i=0; i<nsamples; i++)
		{
			PulsarX::accumulate_mean(xe.data(), xs.data(), s[i], databuffer.buffer.data()+i*nchans, nchans);

			se += s[i];
			ss += s[i]*s[i];
		}

		double tmp = se*se-ss*nsamples;
		if (tmp != 0)
		{
			for (long int j=0; j<nchans; j++)
			{
				alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
				beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
			}
		}

		for (long int i=0; i<nsamples; i++)
		{
			sstdzero[i] = PulsarX::remove_baseline_reduce(databuffer.buffer.data()+i*nchans, databuffer.buffer.data()+i*nchans, alpha.data(), beta.data(), s[i], nchans);
			sstdzero[i] /= nchans;
		}

		runMedian2(sstdzero.data(), sstd.data(), nsamples, width/tsamp);

		for (long int i=0; i<nsamples; i++)
		{
			double norm = sstd[i] == 0. ? 0. : 1./sstd[i];
			for (long int j=0; j<nchans; j++)
			{
				databuffer.buffer[i*nchans+j] *= norm;
			}
		}
	}
	else
	{
		vector<double> xe(nchans, 0.);
		vector<double> xs(nchans, 0.);
		vector<double> alpha(nchans, 0.);
		vector<double> beta(nchans, 0.);
		vector<double> szero(nsamples, 0.);
		vector<double> sstdzero(nsamples, 0.);
		vector<double> s(nsamples, 0.);
		vector<double> sstd(nsamples, 0.);
		double se = 0.;
		double ss = 0.;

		for (long int i=0; i<nsamples; i++)
		{
			double temp = 0.;
			for (long int j=0; j<nchans; j++)
			{
				temp += databuffer.buffer[i*nchans+j];
			}
			szero[i] = temp/nchans;
		}

		runMedian2(szero.data(), s.data(), nsamples, width/tsamp);

		for (long int i=0; i<nsamples; i++)
		{
			for (long int j=0; j<nchans; j++)
			{
				xe[j] += databuffer.buffer[i*nchans+j];
			}

			se += s[i];
			ss += s[i]*s[i];
			
			for (long int j=0; j<nchans; j++)
			{
				xs[j] += databuffer.buffer[i*nchans+j]*s[i];
			}
		}

		double tmp = se*se-ss*nsamples;
		if (tmp != 0)
		{
			for (long int j=0; j<nchans; j++)
			{
				alpha[j] = (xe[j]*se-xs[j]*nsamples)/tmp;
				beta[j] = (xs[j]*se-xe[j]*ss)/tmp;
			}
		}

		for (long int i=0; i<nsamples; i++)
		{
			for (long int j=0; j<nchans; j++)
			{
				databuffer.buffer[i*nchans+j] = (databuffer.buffer[i*nchans+j]-alpha[j]*s[i]-beta[j]);
				sstdzero[i] +=  databuffer.buffer[i*nchans+j] * databuffer.buffer[i*nchans+j];
			}
			sstdzero[i] /= nchans;
		}

		runMedian2(sstdzero.data(), sstd.data(), nsamples, width/tsamp);

		for (long int i=0; i<nsamples; i++)
		{
			double norm = sstd[i] == 0. ? 0. : 1./sstd[i];
			for (long int j=0; j<nchans; j++)
			{
				databuffer.buffer[i*nchans+j] *= norm;
			}
		}
	}

#endif

	std::fill(databuffer.means.begin(), databuffer.means.end(), 0.);
	std::fill(databuffer.vars.begin(), databuffer.vars.end(), 1.);
	databuffer.mean_var_ready = false;
	databuffer.equalized = false;
	counter += nsamples;

	databuffer.isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return databuffer.get();
}