/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 21:25:22
 * @modify date 2023-09-01 21:25:22
 * @desc [description]
 */

#ifndef DEFARADAY_H
#define DEFARADAY_H

#include "databuffer.h"

class Defaraday : public DataBuffer<float>
{
public:
	Defaraday();
	~Defaraday();

	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter(DataBuffer<float> &databuffer);

public:
	double rm;

private:
	size_t nifs;
	size_t nchans_real;
	std::vector<double> cos_2psi;
	std::vector<double> sin_2psi;
};

#endif /* DEFARADAY_H */
