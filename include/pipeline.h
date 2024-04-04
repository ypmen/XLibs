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
	class Pipeline : DataBuffer<float>
	{
	public:
		enum mode_t {MEMORY, SPEED};

		struct config_t
		{
			mode_t mode = MEMORY;

			//downsample
			int td = 1;
			int fd = 1;

			//baseline
			float bswidth = 0.1;

			//rfi
			std::vector<std::pair<double, double>> zaplist;
			std::vector<std::vector<std::string>> rfilist;
			double bandlimit = 10;
			double widthlimit = 10e-3;
			double bandlimitKT = 10;
			float threKadaneT = 7;
			float threKadaneF = 10;
			float threMask = 7;
			std::string filltype = "mean";
		} config;

	public:
		Pipeline(config_t config);
		~Pipeline();
		void prepare(DataBuffer<float> &databuffer);
		DataBuffer<float> * run(DataBuffer<float> &databuffer);		

	private:
		//components
		Downsample downsample;
		Equalize equalize;
		BaseLine baseline;
		RFI rfi;
	};
}

#endif /* PIPELINE_H */
