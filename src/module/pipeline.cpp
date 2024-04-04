/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-04 10:17:17
 * @modify date 2024-04-04 10:17:17
 * @desc [description]
 */

#include "pipeline.h"

using namespace XLIBS;

Pipeline::Pipeline(mode_t mode) : _mode(mode)
{
	td = 1;
	fd = 1;

	bswidth = 0.1;

	threMask = 7;
	bandlimit = 10;
	bandlimitKT = 10.;
	threKadaneT = 7;
	threKadaneF = 10;
	widthlimit = 10e-3;
	filltype = "mean";
}

Pipeline::~Pipeline()
{

}

void Pipeline::prepare(DataBuffer<float> &databuffer)
{
	downsample.td = td;
	downsample.fd = fd;
	downsample.prepare(databuffer);

	equalize.prepare(downsample);

	baseline.width = bswidth;
	baseline.prepare(equalize);

	rfi.filltype = filltype;
	rfi.prepare(baseline);

	if (_mode == MEMORY)
	{
		downsample.close();
		downsample.closable = true;

		equalize.close();
		equalize.closable = true;

		baseline.close();
		baseline.closable = true;

		rfi.close();
		rfi.closable = true;
	}
}

DataBuffer<float> * Pipeline::run(DataBuffer<float> &databuffer)
{
	DataBuffer<float> *data = downsample.run(databuffer);
	
	data = equalize.filter(*data);

	data = baseline.filter(*data);

	data = rfi.zap(*data, zaplist);
	if (rfi.isbusy) rfi.closable = false;
	
	for (auto irfi = rfilist.begin(); irfi!=rfilist.end(); ++irfi)
	{
		if ((*irfi)[0] == "mask")
		{
			data = rfi.mask(*data, threMask, stoi((*irfi)[1]), stoi((*irfi)[2]));
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "kadaneF")
		{
			data = rfi.kadaneF(*data, threKadaneF*threKadaneF, widthlimit, stoi((*irfi)[1]), stoi((*irfi)[2]));
			if (rfi.isbusy) rfi.closable = false;
		}
		else if ((*irfi)[0] == "kadaneT")
		{
			data = rfi.kadaneT(*data, threKadaneT*threKadaneT, bandlimitKT, stoi((*irfi)[1]), stoi((*irfi)[2]));
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

	if (!databuffer.isbusy && _mode == MEMORY) data->closable = true;
}