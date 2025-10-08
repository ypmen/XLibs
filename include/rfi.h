/*
 * rfi.h
 *
 *  Created on: May 6, 2020
 *      Author: ypmen
 */

#ifndef RFI_H
#define RFI_H

#include <string>
#include <vector>
#include <utility>

#include "databuffer.h"
#include "equalize.h"

using namespace std;

class RFI : public DataBuffer<float>
{
public:
	RFI();
	RFI(nlohmann::json &config);
	RFI(const RFI &rfi);
	RFI & operator=(const RFI &rfi);
	~RFI();
	void read_config(nlohmann::json &config);
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	DataBuffer<float> * zap(DataBuffer<float> &databuffer, const vector<pair<double, double>> &zaplist);
	DataBuffer<float> * zap_by_channel(DataBuffer<float> &databuffer, const std::vector<int> &zaplist);
	DataBuffer<float> * zdot(DataBuffer<float> &databuffer);
	DataBuffer<float> * zero(DataBuffer<float> &databuffer);
	DataBuffer<float> * mask(DataBuffer<float> &databuffer, float threRFI2, int td, int fd);
	DataBuffer<float> * kadaneF(DataBuffer<float> &databuffer, float threRFI2, double widthlimit, int td, int fd);
	DataBuffer<float> * kadaneT(DataBuffer<float> &databuffer, float threRFI2, double bandlimit, int td, int fd);
	DataBuffer<float> * get(){return this;}
public:
	string filltype;
	std::vector<std::pair<double, double>> zaplist;
	std::vector<int> zaplist_channel;
	std::vector<std::vector<std::string>> rfilist;
	float thremask;
	float threKadaneF;
	float threKadaneT;
	double widthlimit;
	double bandlimitKT;
private:
	Equalize equalize;
};


#endif /* RFI_H */
