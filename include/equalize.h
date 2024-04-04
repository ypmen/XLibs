/*
 * equalize.h
 *
 *  Created on: May 5, 2020
 *      Author: ypmen
 */

#ifndef EQUALIZE_H_
#define EQUALIZE_H_

#include "databuffer.h"

class Equalize : public DataBuffer<float>
{
public:
	Equalize();
	Equalize(nlohmann::json &config){};
	Equalize(const Equalize &equalize);
	Equalize & operator=(const Equalize &equalize);
	~Equalize();
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter(DataBuffer<float> &databuffer);
	DataBuffer<float> * run(DataBuffer<float> &databuffer);
	DataBuffer<float> * get(){return this;}
};

#endif /* EQUALIZE_H_ */