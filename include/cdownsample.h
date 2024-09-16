/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2022-10-18 11:46:29
 * @modify date 2022-10-18 11:46:29
 * @desc [description]
 */

#ifndef CDOWNSAMPLE_H
#define CDOWNSAMPLE_H

#include "databuffer.h"

class cDownsample : public DataBuffer<std::complex<float>>
{
public:
	cDownsample();
	cDownsample(const cDownsample &cdownsample);
	cDownsample & operator=(const cDownsample &cdownsample);
	cDownsample(int tds, int fds);
	~cDownsample();
	void prepare(DataBuffer<std::complex<float>> &databuffer);
	DataBuffer<std::complex<float>> * run(DataBuffer<std::complex<float>> &databuffer);
	DataBuffer<std::complex<float>> * get(){return this;}
public:
	int td;
	int fd;
};


#endif /* CDOWNSAMPLE_H */
