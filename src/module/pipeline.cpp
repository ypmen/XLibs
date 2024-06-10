/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-04 10:17:17
 * @modify date 2024-04-04 10:17:17
 * @desc [description]
 */

#include "pipeline.h"

using namespace XLIBS;

Pipeline::Pipeline(nlohmann::json &config_downsample, nlohmann::json &config_equalize, nlohmann::json &config_baseline, nlohmann::json &config_rfi, mode_t mode) : downsample(config_downsample), equalize(config_equalize), baseline(config_baseline), rfi(config_rfi), mode(mode)
{
}

Pipeline::~Pipeline()
{

}

void Pipeline::prepare(DataBuffer<float> &databuffer)
{
	downsample.prepare(databuffer);

	equalize.prepare(downsample);

	baseline.prepare(equalize);

	rfi.prepare(baseline);

	DataBuffer<float>::prepare(rfi);

	if (mode == MEMORY)
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

	data = rfi.run(*data);
	
	if (!databuffer.isbusy && mode == MEMORY) data->closable = true;

	return DataBuffer<float>::filter(*data);
}