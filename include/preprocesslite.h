/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2021-06-01 18:07:34
 * @modify date 2021-06-01 18:07:34
 * @desc [description]
 */

#ifndef PREPROCESSLITE_H
#define PREPROCESSLITE_H

#include "databuffer.h"

class PreprocessLite : public DataBuffer<float>
{
public:
	PreprocessLite()
	{
		td = 1;
		fd = 1;
		thresig = 3.;
		filltype = "mean";
		killrate = 0.;
	}
	PreprocessLite(nlohmann::json &config)
	{
		td = config["td"];
		fd = config["fd"];
		thresig = config["zapthre"];
		filltype = config["filltype"];
		killrate = 0.;
	}
	~PreprocessLite(){}
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	DataBuffer<float> * get(){return this;}
public:
	int td;
	int fd;
	float thresig;
	string filltype;
	float killrate;

	std::vector<pair<double, double>> zaplist;
	std::vector<std::vector<unsigned char>> mask;
};

#endif /* PREPROCESSLITE_H */
