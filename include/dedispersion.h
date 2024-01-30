/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 22:35:50
 * @modify date 2023-09-01 22:35:50
 * @desc [description]
 */

#ifndef DEDISPERSION_H
#define DEDISPERSION_H

#include "databuffer.h"

class Dedispersion : public DataBuffer<float>
{
public:
	Dedispersion();
	~Dedispersion();
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);

public:
	double dm;

public:
	size_t offset;

private:
	std::vector<float> buf;
	size_t buf_size;
	std::vector<int> delayn;

public:
	static double dmdelay(double dm, double fh, double fl)
	{
		return 1./2.41e-4*dm*(1./(fl*fl)-1./(fh*fh));
	}
};

#endif /* DEDISPERSION_H */
