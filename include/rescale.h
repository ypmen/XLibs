/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 21:11:46
 * @modify date 2023-09-01 21:11:46
 * @desc [description]
 */

#ifndef RESCALE_H
#define RESCALE_H

#include "databuffer.h"

class Rescale : public DataBuffer<float>
{
public:
	Rescale();
	~Rescale();

	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter(DataBuffer<float> &databuffer);

public:
	std::vector<float> chmean;
	std::vector<float> chstd;
	std::vector<float> chweight;
};

#endif /* RESCALE_H */
