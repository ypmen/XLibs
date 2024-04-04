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

DADAreader::DADAreader(const std::string &key) : reader(key)
{
	nifs = 1;
	nbits = 8;
}

DADAreader::~DADAreader()
{
}

void DADAreader::read_header()
{
	BOOST_LOG_TRIVIAL(info)<<"read header";

	nlohmann::json header;
	reader.prepare(header);

	telescope = header["telescope"];
	source_name = header["source_name"];
	ra = header["ra"];
	dec = header["dec"];
	beam = header["beam"];
	std::string tstart = header["tstart"];
	start_mjd.format(std::stod(tstart));
	nifs = header["nifs"];
	nbits = header["nbits"];
	nchans = header["nchans"];
	tsamp = header["tsamp"];
	
	double fch1 = header["fch1"];
	double foff = header["foff"];
	for (size_t j=0; j<nchans; j++)
	{
		frequencies[j] = fch1 + j * foff;
	}
}

size_t DADAreader::read_data(DataBuffer<float> &databuffer, size_t ns)
{
	assert(databuffer.buffer.size() > 0);

	uint64_t bytes = 0;
	switch (nbits)
	{
	case 8:
	{
		std::vector<unsigned char> temp(ns * nchans, 0);

		bytes = reader.run((char *)(temp.data()), ns * nchans * sizeof(unsigned char));

		if (nifs == 1)
		{
			for (size_t i=0; i<ns; i++)
			{
				for (size_t j=0; j<nchans; j++)
				{
					databuffer.buffer[i * nchans + j] = temp[i * nchans + j];
				}
			}
		}
		else
		{
			for (size_t i=0; i<ns; i++)
			{
				for (size_t j=0; j<nchans; j++)
				{
					unsigned char xx = temp[i * nifs * nchans + 0 * nchans + j];
					unsigned char yy = temp[i * nifs * nchans + 1 * nchans + j];
					databuffer.buffer[i * nchans + j] = xx + yy;
				}
			}
		}

	}; break;
	
	default:
	{
		BOOST_LOG_TRIVIAL(error)<<"data type is not supported";
		exit(-1);
	}; break;
	}

	if (bytes != ns * nchans * nbits / 8)
	{
		BOOST_LOG_TRIVIAL(warning)<<"size mismatch";
	}
	
	return bytes * nbits / 8;
}