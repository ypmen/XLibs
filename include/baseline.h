/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2021-01-17 12:56:51
 * @modify date 2021-01-17 12:56:51
 * @desc [description]
 */

#ifndef BASELINE_H
#define BASELINE_H

#include "databuffer.h"

class BaseLine : public DataBuffer<float>
{
public:
	BaseLine();
	BaseLine(nlohmann::json &config);
	~BaseLine();
	void read_config(nlohmann::json &config);
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter2(DataBuffer<float> &databuffer);
	DataBuffer<float> * get(){return this;}
public:
	float width;
	std::vector<double> outref;
};

#endif /* BASELINE_H */
