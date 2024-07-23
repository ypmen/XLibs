/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-10 20:27:24
 * @modify date 2024-04-10 20:27:24
 * @desc [description]
 */

#ifndef STAT2_H
#define STAT2_H

#include "databuffer.h"

class Stat2
{
public:
	Stat2();
	~Stat2();
	void prepare(DataBuffer<unsigned char> &databuffer);
	void run(DataBuffer<unsigned char> &databuffer);

public:
	std::vector<double> frequencies;
	std::vector<int> hists;
};


#endif /* STAT2_H */
