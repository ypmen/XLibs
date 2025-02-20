#ifndef FLIP_H
#define FLIP_H

#include "databuffer.h"

class Flip : public DataBuffer<float>
{
public:
	Flip();
	~Flip();
	void prepare(DataBuffer<float> &databuffer);
	DataBuffer<float> * filter(DataBuffer<float> &databuffer);
	DataBuffer<float> * get(){return this;}
};


#endif /* FLIP_H */
