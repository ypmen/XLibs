/**
 * @author Yunpeng Men
 * @email ypmen@pku.edu.cn
 * @create date 2020-08-11 11:38:57
 * @modify date 2020-08-11 11:38:57
 * @desc [description]
 */

#include "databuffer.h"

#include <fstream>
#include <string.h>
#include <fftw3.h>

using namespace std;

/* (nsamples, nchans) */

template <typename T>
DataBuffer<T>::DataBuffer()
{
	equalized = false;
	mean_var_ready = false;
	isbusy = false;
	closable = false;
	counter = 0;
	nsamples = 0;
	tsamp = 0.;
	nchans = 0;
}

template <typename T>
DataBuffer<T>::DataBuffer(const DataBuffer<T> &databuffer)
{
	equalized = databuffer.equalized;
	mean_var_ready = databuffer.mean_var_ready;
	isbusy = databuffer.isbusy;
	closable = databuffer.closable;
	counter = databuffer.counter;
	nsamples = databuffer.nsamples;
	tsamp = databuffer.tsamp;
	nchans = databuffer.nchans;
	frequencies = databuffer.frequencies;

	means = databuffer.means;
	vars = databuffer.vars;
	weights = databuffer.weights;

	buffer = databuffer.buffer;
}

template <typename T>
DataBuffer<T> & DataBuffer<T>::operator=(const DataBuffer<T> &databuffer)
{
	equalized = databuffer.equalized;
	mean_var_ready = databuffer.mean_var_ready;
	isbusy = databuffer.isbusy;
	closable = databuffer.closable;
	counter = databuffer.counter;
	nsamples = databuffer.nsamples;
	tsamp = databuffer.tsamp;
	nchans = databuffer.nchans;
	frequencies = databuffer.frequencies;

	means = databuffer.means;
	vars = databuffer.vars;
	weights = databuffer.weights;

	buffer = databuffer.buffer;

	return *this;    
}

template <typename T>
DataBuffer<T>::DataBuffer(long int ns, int nc)
{
	counter = 0;
	resize(ns, nc);
	tsamp = 0.;

	means.resize(nc, 0.);
	vars.resize(nc, 0.);
	weights.resize(nc, 0.);

	equalized = false;
	isbusy = false;
	closable = false;
}

template <typename T>
DataBuffer<T>::~DataBuffer(){}

template <typename T>
void DataBuffer<T>::prepare(DataBuffer<T> &databuffer)
{
	equalized = databuffer.equalized;
	nsamples = databuffer.nsamples;
	nchans = databuffer.nchans;

	resize(nsamples, nchans);

	tsamp = databuffer.tsamp;
	frequencies = databuffer.frequencies;

	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	weights.resize(nchans, 0.);
}

template <typename T>
DataBuffer<T> * DataBuffer<T>::run(DataBuffer<T> &databuffer)
{
	if (closable) open();

	buffer = databuffer.buffer;

	means = databuffer.means;
	vars = databuffer.vars;
	weights = databuffer.weights;

	mean_var_ready = databuffer.mean_var_ready;

	counter += nsamples;

	databuffer.isbusy = false;
	isbusy = true;

	if (databuffer.closable) databuffer.close();

	return this;
};

template <typename T>
DataBuffer<T> * DataBuffer<T>::filter(DataBuffer<T> &databuffer)
{
	counter += nsamples;

	databuffer.isbusy = true;

	return databuffer.get();
};

template <typename T>
void DataBuffer<T>::open()
{
	buffer.clear();
	buffer.resize(nsamples*nchans, 0.);
}

template <typename T>
void DataBuffer<T>::close()
{
	buffer.clear();
	buffer.shrink_to_fit();
}

template <typename T>
void DataBuffer<T>::dump2txt(const string fname)
{
	ofstream outfile;
	outfile.open(fname, ofstream::out);

	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			outfile<<buffer[i*nchans+j]<<" ";
		}
		outfile<<endl;
	}

	outfile.close();
}

template <typename T>
void DataBuffer<T>::dump2bin(const string fname)
{
	ofstream outfile;
	outfile.open(fname, ofstream::binary);

	outfile.write((char *)(&buffer[0]), sizeof(T)*nsamples*nchans);

	outfile.close();
}

template <typename T>
void DataBuffer<T>::dump(const string fname)
{
	ofstream outfile;
	outfile.open(fname, ios::binary|ios::app);

	outfile.write((char *)(&buffer[0]), sizeof(T)*nsamples*nchans);

	outfile.close();
}

template <typename T>
void DataBuffer<T>::resize(long int ns, int nc)
{
	nsamples = ns;
	nchans = nc;
	buffer.resize(nsamples*nchans, 0.);
	frequencies.resize(nchans, 0.);
}

template <typename T>
void DataBuffer<T>::get_mean_rms(vector<T> &mean, vector<T> &var)
{
	mean.resize(nchans, 0.);
	var.resize(nchans, 0.);
	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			mean[j] += buffer[i*nchans+j];
			var[j] += buffer[i*nchans+j]*buffer[i*nchans+j];
		}
	}

	for (long int j=0; j<nchans; j++)
	{
		mean[j] /= nsamples;
		var[j] /= nsamples;
		var[j] -= mean[j]*mean[j];
	}
}

template <>
void DataBuffer<float>::get_mean_rms()
{
	means.resize(nchans, 0.);
	vars.resize(nchans, 0.);
	for (long int i=0; i<nsamples; i++)
	{
		for (long int j=0; j<nchans; j++)
		{
			means[j] += buffer[i*nchans+j];
			vars[j] += buffer[i*nchans+j]*buffer[i*nchans+j];
		}
	}

	for (long int j=0; j<nchans; j++)
	{
		means[j] /= nsamples;
		vars[j] /= nsamples;
		vars[j] -= means[j]*means[j];

		if (vars[j] == 0.) weights[j] = 0.;
	}

	mean_var_ready = true;
}

template class DataBuffer<char>;
template class DataBuffer<unsigned char>;
template class DataBuffer<short>;
template class DataBuffer<float>;
template class DataBuffer<double>;
template class DataBuffer<complex<float>>;
template class DataBuffer<complex<double>>;