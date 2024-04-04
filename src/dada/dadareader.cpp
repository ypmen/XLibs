/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-03 19:44:51
 * @modify date 2024-04-03 19:44:51
 * @desc [description]
 */

#include "dadareader.h"
#include "json.hpp"

#include "logging.h"

using namespace PSRDADA;

template <typename T>
DADAreader<T>::DADAreader(const std::string &key) : reader(key)
{
}

template <typename T>
DADAreader<T>::~DADAreader()
{
}

template <typename T>
bool DADAreader<T>::prepare()
{
	nlohmann::json header;
	reader.init(header);

	// initialize databuffer
	DataBuffer<T>::nsamples = header["nsamples"];
	DataBuffer<T>::nchans = header["nchans"];

	DataBuffer<T>::resize(DataBuffer<T>::nsamples, DataBuffer<T>::nchans);

	DataBuffer<T>::tsamp = header["tsamp"];
	
	double fch1 = header["fch1"];
	double foff = header["foff"];
	for (size_t j=0; j<DataBuffer<T>::nchans; j++)
	{
		DataBuffer<T>::frequencies[j] = fch1 + j * foff;
	}

	DataBuffer<T>::means.resize(DataBuffer<T>::nchans, 0.);
	DataBuffer<T>::vars.resize(DataBuffer<T>::nchans, 0.);
	DataBuffer<T>::weights.resize(DataBuffer<T>::nchans, 1.);
}

template <typename T>
DataBuffer<T> * DADAreader<T>::run()
{
	BOOST_LOG_TRIVIAL(debug)<<"read dada buffer";

	if (DataBuffer<T>::closable) DataBuffer<T>::open();

	uint64_t bytes = reader.run(DataBuffer<T>::buffer.data(), DataBuffer<T>::nsamples * DataBuffer<T>::nchans * sizeof(T));
	if (bytes != DataBuffer<T>::nsamples * DataBuffer<T>::nchans * sizeof(T))
	{
		BOOST_LOG_TRIVIAL(warning)<<"size mismatch";
	}

	DataBuffer<T>::equalized = false;
	DataBuffer<T>::counter += DataBuffer<T>::nsamples;
	DataBuffer<T>::isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}

template class DADAreader<char>;