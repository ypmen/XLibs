/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 20:04:12
 * @modify date 2023-09-01 20:04:12
 * @desc [description]
 */

#include "stat.h"

#include "logging.h"

Stat::Stat()
{
	zap_threshold = 3.;
}

Stat::~Stat()
{
}

void Stat::prepare(DataBuffer<float> &databuffer)
{
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	resize(nsamples, nchans);

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	chmean.resize(nchans, 0.);
	chstd.resize(nchans, 0.);
	chskewness.resize(nchans, 0.);
	chkurtosis.resize(nchans, 0.);
	chcorr.resize(nchans, 0.);
	chweight.resize(nchans, 0.);

	chmean1.resize(nchans, 0.);
	chmean2.resize(nchans, 0.);
	chmean3.resize(nchans, 0.);
	chmean4.resize(nchans, 0.);
	last_data.resize(nchans, 0.);
}

DataBuffer<float> * Stat::run(DataBuffer<float> &databuffer)
{
	BOOST_LOG_TRIVIAL(debug)<<"perform stat";

	for (size_t i=0; i<databuffer.nsamples; i++)
	{
		for (size_t j=0; j<databuffer.nchans; j++)
		{
			double tmp1 = databuffer.buffer[i * databuffer.nchans + j];
			double tmp2 = tmp1*tmp1;
			double tmp3 = tmp2*tmp1;
			double tmp4 = tmp2*tmp2;
			chmean1[j] += tmp1;
			chmean2[j] += tmp2;
			chmean3[j] += tmp3;
			chmean4[j] += tmp4;

			chcorr[j] += tmp1*last_data[j];
			last_data[j] = tmp1;
		}
	}

	counter += nsamples;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}

void Stat::get_stat()
{
	for (long int j=0; j<nchans; j++)
	{
		chmean1[j] /= counter;
		chmean2[j] /= counter;
		chmean3[j] /= counter;
		chmean4[j] /= counter;

		chcorr[j] /= counter - 1;

		double tmp = chmean1[j]*chmean1[j];

		chmean[j] = chmean1[j];
		chstd[j] = chmean2[j]-tmp;
		
		if (chstd[j] > 0.)
		{
			chskewness[j] = chmean3[j]-3.*chmean2[j]*chmean1[j]+2.*tmp*chmean1[j];
			chkurtosis[j] = chmean4[j]-4.*chmean3[j]*chmean1[j]+6.*chmean2[j]*tmp-3.*tmp*tmp;

			chkurtosis[j] /= chstd[j]*chstd[j];
			chkurtosis[j] -= 3.;

			chskewness[j] /= chstd[j]*std::sqrt(chstd[j]);

			chcorr[j] -= tmp;
			chcorr[j] /= chstd[j];
		}
		else
		{
			chstd[j] = 1.;
			chkurtosis[j] = std::numeric_limits<float>::max();
			chskewness[j] = std::numeric_limits<float>::max();

			chcorr[j] = std::numeric_limits<float>::max();
		}

		chstd[j] = std::sqrt(chstd[j]);
	}

	/* calculate mean and std of chkurtosis and chskewness */
	std::vector<float> mean_sort(chmean.begin(), chmean.end());
	std::nth_element(mean_sort.begin(), mean_sort.begin()+mean_sort.size()/4, mean_sort.end(), std::less<float>());
	float mean_q1 = mean_sort[mean_sort.size()/4];
	std::nth_element(mean_sort.begin(), mean_sort.begin()+mean_sort.size()/4, mean_sort.end(), std::greater<float>());
	float mean_q3 =mean_sort[mean_sort.size()/4];
	float mean_R = mean_q3-mean_q1;

	std::vector<float> std_sort(chstd.begin(), chstd.end());
	std::nth_element(std_sort.begin(), std_sort.begin()+std_sort.size()/4, std_sort.end(), std::less<float>());
	float std_q1 = std_sort[std_sort.size()/4];
	std::nth_element(std_sort.begin(), std_sort.begin()+std_sort.size()/4, std_sort.end(), std::greater<float>());
	float std_q3 =std_sort[std_sort.size()/4];
	float std_R = std_q3-std_q1;

	std::vector<float> kurtosis_sort(chkurtosis.begin(), chkurtosis.end());
	std::nth_element(kurtosis_sort.begin(), kurtosis_sort.begin()+kurtosis_sort.size()/4, kurtosis_sort.end(), std::less<float>());
	float kurtosis_q1 = kurtosis_sort[kurtosis_sort.size()/4];
	std::nth_element(kurtosis_sort.begin(), kurtosis_sort.begin()+kurtosis_sort.size()/4, kurtosis_sort.end(), std::greater<float>());
	float kurtosis_q3 =kurtosis_sort[kurtosis_sort.size()/4];
	float kurtosis_R = kurtosis_q3-kurtosis_q1;

	std::vector<float> skewness_sort(chskewness.begin(), chskewness.end());
	std::nth_element(skewness_sort.begin(), skewness_sort.begin()+skewness_sort.size()/4, skewness_sort.end(), std::less<float>());
	float skewness_q1 = skewness_sort[skewness_sort.size()/4];
	std::nth_element(skewness_sort.begin(), skewness_sort.begin()+skewness_sort.size()/4, skewness_sort.end(), std::greater<float>());
	float skewness_q3 =skewness_sort[skewness_sort.size()/4];
	float skewness_R = skewness_q3-skewness_q1;

	std::vector<double> corr_sort(chcorr.begin(), chcorr.end());
	std::nth_element(corr_sort.begin(), corr_sort.begin()+corr_sort.size()/4, corr_sort.end(), std::less<float>());
	double corr_q1 = corr_sort[corr_sort.size()/4];
	std::nth_element(corr_sort.begin(), corr_sort.begin()+corr_sort.size()/4, corr_sort.end(), std::greater<float>());
	double corr_q3 = corr_sort[corr_sort.size()/4];
	double corr_R = corr_q3-corr_q1;

	for (long int j=0; j<nchans; j++)
	{
		if (chmean[j]>=mean_q1-zap_threshold*mean_R && \
			chmean[j]<=mean_q3+zap_threshold*mean_R && \
			chstd[j]>=std_q1-zap_threshold*std_R && \
			chstd[j]<=std_q3+zap_threshold*std_R && \
			chkurtosis[j]>=kurtosis_q1-zap_threshold*kurtosis_R && \
			chkurtosis[j]<=kurtosis_q3+zap_threshold*kurtosis_R && \
			chskewness[j]>=skewness_q1-zap_threshold*skewness_R && \
			chskewness[j]<=skewness_q3+zap_threshold*skewness_R && \
			chcorr[j]>=corr_q1-zap_threshold*corr_R && \
			chcorr[j]<=corr_q3+zap_threshold*corr_R)
		{
			chweight[j] = 1.;
		}
	}

	size_t nchans_real = frequencies.size();
	size_t nifs = nchans / nchans_real;

	if (nifs != 0)
	{
		for (size_t k=0; k<nifs; k++)
		{
			for (size_t j=0; j<nchans_real; j++)
			{
				if (chweight[k * nchans_real + j] == 0.)
				{
					for (size_t l=0; l<nifs; l++)
					{
						chweight[l * nchans_real + j] == 0.;
					}
				}
			}
		}
	}
}