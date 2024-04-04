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
	class Pipeline
	{
	public:
		enum mode_t {MEMORY, SPEED};
	public:
		Pipeline(mode_t mode);
		~Pipeline();
		void prepare(DataBuffer<float> &databuffer);
		DataBuffer<float> * run(DataBuffer<float> &databuffer);
	
	public:

		//downsample
		int td;
		int fd;

		//baseline
		float bswidth;

		//rfi
		std::vector<std::pair<double, double>> zaplist;
		std::vector<std::vector<std::string>> rfilist;
		double bandlimit;
		double widthlimit;
		double bandlimitKT;
		float threKadaneT;
		float threKadaneF;
		float threMask;
		std::string filltype;

	private:
		mode_t _mode;
		//components
		Downsample downsample;
		Equalize equalize;
		BaseLine baseline;
		RFI rfi;
	};
}

#endif /* PIPELINE_H */
