/**
 * @author Yunpeng Men
 * @email ypmen@pku.edu.cn
 * @create date 2020-05-15 21:14:38
 * @modify date 2020-05-15 21:14:38
 * @desc [description]
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <immintrin.h>

#include "psrfitswriter.h"

using namespace std;

template <typename T>
PsrfitsWriter<T>::PsrfitsWriter()
{
    mode = Integration::SEARCH;
    dtype = Integration::UINT8;
    gain = 0.;
    offs = 0;
    rms = 20;
    nsubint_per_file = 128;
    nsamp_per_subint = 1024;
    isubint = 0;
    ifile = 0;
    ichunk = 0;
    ibeam = 1;
    npol = 1;

	obs_mode = "SEARCH";
}

template <typename T>
PsrfitsWriter<T>::~PsrfitsWriter()
{
    if (fits.fptr != NULL)
    {
        fits.close();
    }
}

template <typename T>
void PsrfitsWriter<T>::prepare(DataBuffer<float> &databuffer)
{
    assert(nsamp_per_subint%databuffer.nsamples == 0);

	int nbits = 8;
	if (dtype == Integration::UINT4)
		nbits = 4;

    DataBuffer<T>::nchans = databuffer.nchans;
    DataBuffer<T>::nsamples = nsamp_per_subint;

	if (nbits == 4)
	{
		DataBuffer<T>::resize(DataBuffer<T>::nsamples/(8/nbits), DataBuffer<T>::nchans);
		DataBuffer<T>::nsamples = nsamp_per_subint;
	}
	else
    	DataBuffer<T>::resize(DataBuffer<T>::nsamples, DataBuffer<T>::nchans);

    DataBuffer<T>::frequencies = databuffer.frequencies;
    DataBuffer<T>::tsamp = databuffer.tsamp;

    int nchans_real = databuffer.nchans/npol;

    if (mode == Integration::SEARCH)
    {
        stringstream ss_k;
        ss_k << setw(4) << setfill('0') << ifile+1;
        string s_k = ss_k.str();

        stringstream ss_ibeam;
        ss_ibeam << "M" << setw(2) << setfill('0') << ibeam;
        string s_ibeam = ss_ibeam.str();

        fits.primary.start_mjd = start_mjd;
        strcpy(fits.primary.src_name, src_name.c_str());
        strcpy(fits.primary.ra, ra.c_str());
        strcpy(fits.primary.dec, dec.c_str());
        strcpy(fits.primary.telesop, telescope.c_str());
        strcpy(fits.primary.ibeam, to_string(ibeam).c_str());

        fits.filename = rootname + "_" + s_ibeam + "_" + s_k + ".fits";
        strcpy(fits.primary.obs_mode, obs_mode.c_str());
        
        fits.subint.mode = Integration::SEARCH;
        fits.subint.dtype = dtype;
        
        fits.subint.nbits = sizeof(T);
		if (dtype == Integration::UINT4)
			fits.subint.nbits = 4;
        fits.subint.npol = npol;
        fits.subint.nchan = nchans_real;
        fits.subint.nsblk = DataBuffer<T>::nsamples;
        fits.subint.tbin = DataBuffer<T>::tsamp;

        fits.parse_template(template_file);
        fits.primary.unload(fits.fptr);
        fits.subint.unload_header(fits.fptr);

        it.mode = Integration::SEARCH;
        it.dtype = dtype;
        
        it.nbits = sizeof(T);
		if (dtype == Integration::UINT4)
			it.nbits = 4;

        it.tsubint = DataBuffer<T>::tsamp*DataBuffer<T>::nsamples;
    }
}

template <typename T>
DataBuffer<T> * PsrfitsWriter<T>::run(DataBuffer<float> &databuffer)
{
    if (databuffer.counter <= 0) return this;

    if (mode == Integration::SEARCH)
    {
        long int nchans_real = DataBuffer<T>::nchans/npol;

        int np = npol>2 ? 2:npol;
        if (gain == 0.)
        {
            double mean = 0.;
            double var = 0.;
            for (long int i=0; i<databuffer.nsamples; i++)
            {
                for (long int k=0; k<np; k++)
                {
                    for (long int j=0; j<nchans_real; j++)
                    {
                        mean += databuffer.buffer[i*DataBuffer<T>::nchans+k*nchans_real+j];
                        var += (double)(databuffer.buffer[i*DataBuffer<T>::nchans+k*nchans_real+j])*(double)(databuffer.buffer[i*DataBuffer<T>::nchans+k*nchans_real+j]);
                    }
                }
            }
            mean /= databuffer.nsamples*np*nchans_real;
            var /= databuffer.nsamples*np*nchans_real;
            var -= mean*mean;

			mean = 0.;
			var = 1.;

            gain = rms/sqrt(var);
            offs = mean;
        }

        if (dtype == Integration::UINT8)
        {
#ifndef __AVX2__
            for (long int i=0; i<databuffer.nsamples; i++)
            {
                for (long int k=0; k<npol; k++)
                {
                    if (k<2)
                    {
                        for (long int j=0; j<nchans_real; j++)
                        {
							float tmp = std::round((databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]-offs)*gain+128); 
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real+i*npol*nchans_real+k*nchans_real+j] = tmp>255? 255:tmp;
                        }
                    }
                    else
                    {
                        for (long int j=0; j<nchans_real; j++)
                        {
							float tmp = std::round(databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]*gain+128);
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real+i*npol*nchans_real+k*nchans_real+j] = tmp>255? 255:tmp;
                        }
                    }
                }
            }
#else
            assert(nchans_real%8 == 0);
            __m256 avx_a;
            __m256 avx_min = _mm256_setr_ps(0.,0.,0.,0.,0.,0.,0.,0.);
            __m256 avx_max = _mm256_setr_ps(255.,255.,255.,255.,255.,255.,255.,255.);
            __m256 avx_gain = _mm256_setr_ps(gain,gain,gain,gain,gain,gain,gain,gain);
            __m256 avx_offs = _mm256_setr_ps(offs,offs,offs,offs,offs,offs,offs,offs);
            __m256 avx_128 = _mm256_setr_ps(128,128,128,128,128,128,128,128);
            float *temp = (float *)_mm_malloc(sizeof(float)*nchans_real, 32);
            for (long int i=0; i<databuffer.nsamples; i++)
            {
                for (long int k=0; k<npol; k++)
                {
                    unsigned char *pb = (unsigned char *)(&DataBuffer<T>::buffer[0])+ichunk*databuffer.nsamples*npol*nchans_real+i*npol*nchans_real+k*nchans_real;
                    float *pbuffer = &(databuffer.buffer[0])+i*npol*nchans_real+k*nchans_real;
                    if (k<2)
                    {
                        for (long int j=0; j<nchans_real/8; j++)
                        {
                            avx_a = _mm256_load_ps(pbuffer+j*8);
                            avx_a = _mm256_sub_ps(avx_a, avx_offs);
                            avx_a = _mm256_fmadd_ps(avx_a, avx_gain, avx_128);
							avx_a = _mm256_round_ps(avx_a, 0);
                            avx_a = _mm256_max_ps(avx_a, avx_min);
                            avx_a = _mm256_min_ps(avx_a, avx_max);
                            _mm256_store_ps(temp+j*8, avx_a);
                        }
                    }
                    else
                    {
                        for (long int j=0; j<nchans_real/8; j++)
                        {
                            avx_a = _mm256_load_ps(pbuffer+j*8);
                            avx_a = _mm256_fmadd_ps(avx_a, avx_gain, avx_128);
							avx_a = _mm256_round_ps(avx_a, 0);
                            avx_a = _mm256_max_ps(avx_a, avx_min);
                            avx_a = _mm256_min_ps(avx_a, avx_max);
                            _mm256_store_ps(temp+j*8, avx_a);
                        }
                    }
                    for (long int j=0; j<nchans_real; j++)
                    {
                        pb[j] = temp[j];
                    }
                }
            }
            _mm_free(temp);
#endif
        }
		else if (dtype == Integration::UINT4)
		{
			for (long int i=0; i<databuffer.nsamples; i++)
            {
                for (long int k=0; k<npol; k++)
                {
                    if (k<2)
                    {
                        for (long int j=0; j<nchans_real; j+=2)
                        {
							float tmp_a = std::round((databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]-offs)*gain+8);
							float a = tmp_a>15? 15:tmp_a;
							float tmp_b = std::round((databuffer.buffer[i*npol*nchans_real+k*nchans_real+j+1]-offs)*gain+8);
							float b = tmp_b>15? 15:tmp_b;
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real/2+i*npol*nchans_real/2+k*nchans_real/2+j/2] = b * 16 + a;
                        }
                    }
                    else
                    {
                        for (long int j=0; j<nchans_real; j+=2)
                        {
							float tmp_a = std::round(databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]*gain+8);
							float a = tmp_a>15? 15:tmp_a;
							float tmp_b = std::round(databuffer.buffer[i*npol*nchans_real+k*nchans_real+j+1]*gain+8);
							float b = tmp_b>15? 15:tmp_b;
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real/2+i*npol*nchans_real/2+k*nchans_real/2+j/2] = b * 16 + a;
                        }
                    }
                }
            }
		}
        else if (dtype == Integration::FLOAT)
        {
            for (long int i=0; i<databuffer.nsamples; i++)
            {
                for (long int k=0; k<npol; k++)
                {
                    if (k<2)
                    {
                        for (long int j=0; j<nchans_real; j++)
                        {
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real+i*npol*nchans_real+k*nchans_real+j] = (databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]-offs)*gain;
                        }
                    }
                    else
                    {
                        for (long int j=0; j<nchans_real; j++)
                        {
                            DataBuffer<T>::buffer[ichunk*databuffer.nsamples*npol*nchans_real+i*npol*nchans_real+k*nchans_real+j] = (databuffer.buffer[i*npol*nchans_real+k*nchans_real+j]-offs)*gain;
                        }
                    }
                }
            }
        }
        else
        {
            cerr<<"Error: data type is not supported"<<endl;
        }

        if ((++ichunk)*databuffer.nsamples%nsamp_per_subint == 0)
        {
            ichunk = 0;

            it.load_data(&DataBuffer<T>::buffer[0], npol, nchans_real, DataBuffer<T>::nsamples);

            it.load_frequencies(&DataBuffer<T>::frequencies[0], nchans_real);
            
			it.offs_sub = (isubint + 0.5) * nsamp_per_subint * DataBuffer<T>::tsamp;

            fits.subint.unload_integration(fits.fptr, it);
            
            if (++isubint == nsubint_per_file)
            {
                fits.close();
                isubint = 0;
                ifile++;

                stringstream ss_k;
                ss_k << setw(4) << setfill('0') << ifile+1;
                string s_k = ss_k.str();

                stringstream ss_ibeam;
                ss_ibeam << "M" << setw(2) << setfill('0') << ibeam;
                string s_ibeam = ss_ibeam.str();

                fits.primary.start_mjd = start_mjd+databuffer.nsamples*DataBuffer<T>::tsamp;

                fits.filename = rootname + "_" + s_ibeam + "_" + s_k + ".fits";
                strcpy(fits.primary.obs_mode, "SEARCH");

                fits.parse_template(template_file);
                fits.primary.unload(fits.fptr);
                fits.subint.unload_header(fits.fptr);
            }
        }
    }
    
    start_mjd += databuffer.nsamples*DataBuffer<T>::tsamp;

    return this;
}

template class PsrfitsWriter<unsigned char>;
template class PsrfitsWriter<float>;
