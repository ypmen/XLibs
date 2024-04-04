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

template <typename Input_t, typename Output_t>
DADAreader<Input_t, Output_t>::DADAreader(const std::string &key) : reader(key)
{
}

template <typename Input_t, typename Output_t>
DADAreader<Input_t, Output_t>::~DADAreader()
{
}

template <typename Input_t, typename Output_t>
bool DADAreader<Input_t, Output_t>::prepare()
{
	nlohmann::json header;
	if (!reader.init(header)) return false;

	// initialize databuffer
	DataBuffer<Output_t>::nsamples = header["nsamples"];
	DataBuffer<Output_t>::nchans = header["nchans"];

	DataBuffer<Output_t>::resize(DataBuffer<Output_t>::nsamples, DataBuffer<Output_t>::nchans);

	DataBuffer<Output_t>::tsamp = header["tsamp"];
	
	double fch1 = header["fch1"];
	double foff = header["foff"];
	for (size_t j=0; j<DataBuffer<Output_t>::nchans; j++)
	{
		DataBuffer<Output_t>::frequencies[j] = fch1 + j * foff;
	}

	DataBuffer<Output_t>::means.resize(DataBuffer<Output_t>::nchans, 0.);
	DataBuffer<Output_t>::vars.resize(DataBuffer<Output_t>::nchans, 0.);
	DataBuffer<Output_t>::weights.resize(DataBuffer<Output_t>::nchans, 1.);

	return true;
}

template <typename Input_t, typename Output_t>
DataBuffer<Output_t> * DADAreader<Input_t, Output_t>::run()
{
	BOOST_LOG_TRIVIAL(debug)<<"read dada buffer";

	if (DataBuffer<Output_t>::closable) DataBuffer<Output_t>::open();

	std::vector<Input_t> temp(DataBuffer<Output_t>::nsamples * DataBuffer<Output_t>::nchans);

	uint64_t bytes = reader.run((char *)(temp.data()), DataBuffer<Output_t>::nsamples * DataBuffer<Output_t>::nchans * sizeof(Input_t));
	if (bytes != DataBuffer<Output_t>::nsamples * DataBuffer<Output_t>::nchans * sizeof(Input_t))
	{
		BOOST_LOG_TRIVIAL(warning)<<"size mismatch";
		return NULL;
	}
	else
	{
		for (size_t i=0; i<DataBuffer<Output_t>::nsamples * DataBuffer<Output_t>::nchans; i++)
		{
			DataBuffer<Output_t>::buffer[i] = temp[i];
		}
	}

	DataBuffer<Output_t>::equalized = false;
	DataBuffer<Output_t>::counter += DataBuffer<Output_t>::nsamples;
	DataBuffer<Output_t>::isbusy = true;

	BOOST_LOG_TRIVIAL(debug)<<"finished";

	return this;
}

template class DADAreader<unsigned char, float>;