/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-04 10:17:17
 * @modify date 2024-04-04 10:17:17
 * @desc [description]
 */

#include "pipeline.h"

using namespace XLIBS;

Pipeline::Pipeline(config_t config) : config(config)
{
}

Pipeline::~Pipeline()
{

}

void Pipeline::prepare(DataBuffer<float> &databuffer)
{
	downsample.td = config.td;
	downsample.fd = config.fd;
	downsample.prepare(databuffer);

	equalize.prepare(downsample);

	baseline.width = config.bswidth;
	baseline.prepare(equalize);

	rfi.filltype = config.filltype;
	rfi.prepare(baseline);

	DataBuffer<float>::prepare(rfi);

	if (config.mode == MEMORY)
	{
		downsample.close();
		downsample.closable = true;

		equalize.close();
		equalize.closable = true;

		baseline.close();
		baseline.closable = true;

		rfi.close();
		rfi.closable = true;

		DataBuffer<float>::close();
		DataBuffer<float>::closable = true;
	}
}

DataBuffer<float> * Pipeline::run(DataBuffer<float> &databuffer)
{
	DataBuffer<float> *data = downsample.run(databuffer);
	
	data = equalize.filter(*data);

	data = baseline.filter(*data);

	data = rfi.zap(*data, config.zaplist);
	if (rfi.isbusy) rfi.closable = false;
	
	for (auto irfi = config.rfilist.begin(); irfi!=config.rfilist.end(); ++irfi)
	{
		if ((*irfi)[0] == "mask")
		{
			data = rfi.mask(*data, config.threMask, stoi((*irfi)[1]), stoi((*irfi)[2]));
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "kadaneF")
		{
			data = rfi.kadaneF(*data, config.threKadaneF*config.threKadaneF, config.widthlimit, stoi((*irfi)[1]), stoi((*irfi)[2]));
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "kadaneT")
		{
			data = rfi.kadaneT(*data, config.threKadaneT*config.threKadaneT, config.bandlimitKT, stoi((*irfi)[1]), stoi((*irfi)[2]));
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "zdot")
		{
			data = rfi.zdot(*data);
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "zero")
		{
			data = rfi.zero(*data);
			if (rfi.isbusy) rfi.closable = false;
		}
	}

	if (!databuffer.isbusy && config.mode == MEMORY) data->closable = true;

	return DataBuffer<float>::filter(rfi);
}