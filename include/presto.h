#ifndef PRESTO_H
#define PRESTO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#ifdef __AVX2__
#include <boost/align/aligned_allocator.hpp>
#endif

namespace PRESTO 
{
	class Info
	{
	public:
		Info()
		{
			telescope = "unset";
			instrument = "unset";
			beam = "0";
			source_name = "unset";
			ra = "unset";
			dec = "unset";
			observer = "unset";
			epoch = 0.;
			bary = false;
			nsamples = 0;
			tsamp = 0.;
			breaks = false;
			type_obs = "unset";
			beam_diameter = 3600.;
			dm = 0.;
			fch1 = 0.;
			bw = 0.;
			nchans = 0;
			foff = 0.;
			mean = 0.;
			stddev = 0.;
			who = "unset";

			is_end = false;
			skip_start = 0;
			skip_end = 0;
			isample_cur = 0;
			verbose = false;
		}

		~Info(){
			if (datfile.is_open())
			{
				datfile.close();
			}
		}

		void read(const std::string &fname)
		{
			std::ifstream inf_file(fname);
			std::string line;

			while (getline(inf_file, line))
			{
				std::vector<std::string> items;

				boost::split(items, line, boost::is_any_of("="), boost::token_compress_on);

				std::string key = items.front();
				boost::algorithm::trim(key);
				std::string value = items.back();
				boost::algorithm::trim(value);

				if (key.rfind("Data file name without suffix", 0) == 0)
				{
					rootname = value;
				}
				else if (key.rfind("Telescope used", 0) == 0)
				{
					telescope = value;
				}
				else if (key.rfind("Instrument used", 0) == 0)
				{
					instrument = value;
				}
				else if (key.rfind("Object being observed", 0) == 0)
				{
					source_name = value;
				}
				else if (key.rfind("J2000 Right Ascension", 0) == 0)
				{
					ra = value;
				}
				else if (key.rfind("J2000 Declination", 0) == 0)
				{
					dec = value;
				}
				else if (key.rfind("Data observed by", 0) == 0)
				{
					observer = value;
				}
				else if (key.rfind("Epoch of observation (MJD)", 0) == 0)
				{
					epoch = std::stold(value);
				}
				else if (key.rfind("Barycentered?", 0) == 0)
				{
					//bary = std::stoi(value);
				}
				else if (key.rfind("Number of bins in the time series", 0) == 0)
				{
					nsamples = std::stoul(value);
				}
				else if (key.rfind("Width of each time series bin (sec)", 0) == 0)
				{
					tsamp = std::stod(value);
				}
				else if (key.rfind("Any breaks in the data?", 0) == 0)
				{
					//breaks = std::stoi(value);
				}
				else if (key.rfind("Type of observation (EM band)", 0) == 0)
				{
					type_obs = value;
				}
				else if (key.rfind("Beam diameter (arcsec)", 0) == 0)
				{
					beam_diameter = std::stod(value);
				}
				else if (key.rfind("Dispersion measure", 0) == 0)
				{
					dm = std::stod(value);
				}
				else if (key.rfind("Central freq of low channel", 0) == 0)
				{
					fch1 = std::stod(value);
				}
				else if (key.rfind("Total bandwidth (MHz)", 0) == 0)
				{
					bw = std::stod(value);
				}
				else if (key.rfind("Number of channels", 0) == 0)
				{
					nchans = std::stoul(value);
				}
				else if (key.rfind("Channel bandwidth (MHz)", 0) == 0)
				{
					foff = std::stod(value);
				}
				else if (key.rfind("Data analyzed by", 0) == 0)
				{
					who = value;
				}
				else if (key.rfind("Mean", 0) == 0)
				{
					mean = std::stod(value);
				}
				else if (key.rfind("Standard deviation", 0) == 0)
				{
					stddev = std::stod(value);
				}
				else if (key.rfind("Any additional notes", 0) == 0)
				{
					while (getline(inf_file, line))
					{
						notes += line + '\n';
					}
				}
			}
		}

		void write()
		{
			std::ofstream inf_file;
			inf_file.open(rootname+".inf");
			inf_file<<" Data file name without suffix          =  "<<rootname<<std::endl;
			inf_file<<" Telescope used                         =  "<<telescope<<std::endl;
			inf_file<<" Instrument used                        =  "<<instrument<<std::endl;
			inf_file<<" Object being observed                  =  "<<source_name<<std::endl;
			inf_file<<" J2000 Right Ascension (hh:mm:ss.ssss)  =  "<<ra<<std::endl;
			inf_file<<" J2000 Declination     (dd:mm:ss.ssss)  =  "<<dec<<std::endl;
			inf_file<<" Data observed by                       =  "<<observer<<std::endl;
			inf_file<<" Epoch of observation (MJD)             =  "<<std::fixed<<std::setprecision(15)<<epoch<<std::endl;
			inf_file<<" Barycentered?           (1=yes, 0=no)  =  "<<(bary?1:0)<<std::endl;
			inf_file<<" Number of bins in the time series      =  "<<nsamples<<std::endl;
			inf_file<<" Width of each time series bin (sec)    =  "<<std::fixed<<std::setprecision(17)<<tsamp<<std::endl;
			inf_file<<" Any breaks in the data? (1=yes, 0=no)  =  "<<(breaks?1:0)<<std::endl;
			inf_file<<" Type of observation (EM band)          =  "<<type_obs<<std::endl;
			inf_file<<" Beam diameter (arcsec)                 =  "<<beam_diameter<<std::endl;
			inf_file<<" Dispersion measure (cm-3 pc)           =  "<<dm<<std::endl;
			inf_file<<" Central freq of low channel (Mhz)      =  "<<fch1<<std::endl;
			inf_file<<" Total bandwidth (Mhz)                  =  "<<bw<<std::endl;
			inf_file<<" Number of channels                     =  "<<nchans<<std::endl;
			inf_file<<" Channel bandwidth (Mhz)                =  "<<foff<<std::endl;
			inf_file<<" Mean                                   =  "<<mean<<std::endl;
			inf_file<<" Standard deviation                     =  "<<stddev<<std::endl;
			inf_file<<" Data analyzed by                       =  "<<who<<std::endl;
			inf_file.close();
		}

		void open(std::string &fname)
		{
			datfile.open(fname, std::ios::binary);
			if (!datfile.is_open()) {
				std::cerr << "Error opening file: " << fname << std::endl;
				exit(1);
			}
		}
		void skip_head()
		{
			datfile.seekg(skip_start*sizeof(float), std::ios::beg);
			isample_cur = skip_start;
		}
#ifdef __AVX2__
		int read_data(std::vector<float, boost::alignment::aligned_allocator<float, 32>> &data, int n)
#else
		int read_data(std::vector<float> &data, int n)
#endif
		{
			datfile.read((char *)(data.data()), sizeof(float)*n);
			isample_cur += datfile.gcount() / sizeof(float);

			if (verbose)
			{
				std::cerr<<"\r\rfinish "<<std::setprecision(2)<<std::fixed<<tsamp*isample_cur<<" seconds ";
				std::cerr<<"("<<100.*isample_cur/nsamples<<"%)";
			}

			if (isample_cur >= nsamples-skip_end)
				is_end = true;

			return datfile.gcount() / sizeof(float);
		}
		
		void close()
		{
			datfile.close();
		}
	public:
		std::string rootname;
		std::string telescope;
		std::string instrument;
		std::string beam;
		std::string source_name;
		std::string ra;
		std::string dec;
		std::string observer;
		long double epoch;
		bool bary;
		size_t nsamples;
		double tsamp;
		bool breaks;
		std::string type_obs;
		double beam_diameter;
		double dm;
		double fch1;
		double bw;
		size_t nchans;
		double foff;
		double mean;
		double stddev;
		std::string who;
		std::string notes;

		size_t isample_cur = 0;
		size_t skip_start;
		size_t skip_end;
		std::ifstream datfile;
		bool is_end;
		bool verbose;
	};
}

#endif /* PRESTO_H */
