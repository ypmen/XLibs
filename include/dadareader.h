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
#include "dada_def.h"
#include "dada_hdu.h"
#include "dada.h"

namespace PSRDADA {
	template <typename T>
	class DADAreader : public DataBuffer<T>
	{
	public:
		DADAreader(const std::string &key);
		~DADAreader();
		bool prepare();
		DataBuffer<T> * run();
	private:
		PSRDADA::Reader reader;
	};
}

#endif /* DADAREADER_H */
