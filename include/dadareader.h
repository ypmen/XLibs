/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-03 19:43:56
 * @modify date 2024-04-03 19:43:56
 * @desc "read psrdada buffer to databuffer"
 */

#ifndef DADAREADER_H
#define DADAREADER_H

#include "databuffer.h"
#include "mjd.h"
#include "dada_def.h"
#include "dada_hdu.h"
#include "dada.h"

namespace PSRDADA {
	class DADAreader
	{
	public:
		DADAreader(const std::string &key);
		~DADAreader();
		void read_header();
		size_t read_data(DataBuffer<float> &databuffer, size_t ns);
		size_t get_bufsz(){return reader.get_bufsz();}

	public:
		std::string telescope;
		std::string source_name;
		std::string ra;
		std::string dec;
		std::string beam;

	public:
		MJD start_mjd;
		int nifs;
		int nbits;
		double tsamp;
		size_t nchans;
		std::vector<double> frequencies;

	private:
		PSRDADA::Reader reader;
	};
}

#endif /* DADAREADER_H */
