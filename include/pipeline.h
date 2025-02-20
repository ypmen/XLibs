/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-04 10:07:10
 * @modify date 2024-04-04 10:07:10
 * @desc "downsample - equalize - baseline - rfi"
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include "databuffer.h"
#include "downsample.h"
#include "equalize.h"
#include "baseline.h"
#include "rfi.h"

namespace XLIBS {
	class Pipeline : public DataBuffer<float>
	{
	public:
		enum mode_t {MEMORY, SPEED};

	public:
		Pipeline(nlohmann::json &config_downsample, nlohmann::json &config_equalize, nlohmann::json &config_baseline, nlohmann::json &config_rfi, mode_t mode = MEMORY);
		~Pipeline();
		void prepare(DataBuffer<float> &databuffer);
		DataBuffer<float> * run(DataBuffer<float> &databuffer);		

	private:
		mode_t mode;
		//components
		Downsample downsample;
		Equalize equalize;
		BaseLine baseline;
		RFI rfi;
	};
}

#endif /* PIPELINE_H */
