/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2023-09-01 19:52:39
 * @modify date 2023-09-01 19:52:39
 * @desc [description]
 */

#ifndef STAT_H
#define STAT_H

#include "databuffer.h"

class Stat : public DataBuffer<float>
{
public:
	Stat();
	~Stat();
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	void get_stat();

public:
	float zap_threshold;

public:
	std::vector<float> chmean;
	std::vector<float> chstd;
	std::vector<float> chskewness;
	std::vector<float> chkurtosis;
	std::vector<float> chweight;
	std::vector<float> chcorr;

private:
	std::vector<double> chmean1;
	std::vector<double> chmean2;
	std::vector<double> chmean3;
	std::vector<double> chmean4;
	std::vector<double> last_data;
};

#endif /* STAT_H */
