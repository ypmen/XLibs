/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2022-08-20 23:10:30
 * @modify date 2022-08-20 23:10:30
 * @desc [description]
 */

#include "psrfitsreader.h"
#include "logging.h"
#include "mjd.h"
#include "utils.h"

PsrfitsReader::PsrfitsReader()
{
	ntot = 0;
	count = 0;
	ns_psfn = 0;

	ifile_cur = 0;
	isubint_cur = 0;
	isample_cur = 0;

	update_file = true;
	update_subint = true;
}

PsrfitsReader::~PsrfitsReader()
{
}

void PsrfitsReader::check()
{
	size_t npsf = fnames.size();
	BOOST_LOG_TRIVIAL(info)<<"scan "<<npsf<<" psrfits files";

	psf.resize(npsf);
	for (size_t i=0; i<npsf; i++)
	{
		psf[i].filename = fnames[i];
	}

	for (size_t i=0; i<npsf; i++)
	{
		psf[i].open();
		psf[i].primary.load(psf[i].fptr);
		psf[i].load_mode();
		psf[i].subint.load_header(psf[i].fptr);
		nsamples += psf[i].subint.nsamples;

		Integration tmp;
		psf[i].subint.load_integration(psf[i].fptr, 0, tmp);

		mjd_starts.push_back(psf[i].primary.start_mjd + (tmp.offs_sub - 0.5 * psf[i].subint.nsblk * psf[i].subint.tbin));
		mjd_ends.push_back(psf[i].primary.start_mjd + ((tmp.offs_sub - 0.5 * psf[i].subint.nsblk * psf[i].subint.tbin) + psf[i].subint.nsamples*psf[i].subint.tbin));
		psf[i].close();
	}

	// check continuity
	idmap = argsort(mjd_starts);

	for (size_t i=0; i<npsf-1; i++)
	{
		if (abs((mjd_ends[idmap[i]]-mjd_starts[idmap[i+1]]).to_second())>0.5*psf[idmap[i]].subint.tbin)
		{
			if (contiguous)
			{
				BOOST_LOG_TRIVIAL(warning)<<"time not contiguous";
			}
			else
			{
				BOOST_LOG_TRIVIAL(error)<<"time not contiguous";
				exit(-1);
			}
		}
	}
}

void PsrfitsReader::skip_head()
{
	size_t npsf = fnames.size();

	// update ifile, isubint, isample
	for (size_t idxn=ifile_cur; idxn<npsf; idxn++)
	{
		size_t n = idmap[idxn];

		ns_psfn = 0;

		psf[n].open();
		psf[n].primary.load(psf[n].fptr);
		psf[n].load_mode();
		psf[n].subint.load_header(psf[n].fptr);

		for (size_t s=0; s<psf[n].subint.nsubint; s++)
		{
			for (size_t i=0; i<psf[n].subint.nsblk; i++)
			{
				if (count == skip_start)
				{
					ifile_cur = idxn;
					isubint_cur = s;
					isample_cur = i;

					update_file = false;

					return;
				}

				count++;

				if (++ns_psfn == psf[n].subint.nsamples)
				{
					goto next;
				}
			}
		}
		next:
		psf[n].close();
	}
}

void PsrfitsReader::read_header()
{
	BOOST_LOG_TRIVIAL(info)<<"read header";

	psf[idmap[0]].open();
	psf[idmap[0]].primary.load(psf[idmap[0]].fptr);
	psf[idmap[0]].load_mode();
	psf[idmap[0]].subint.load_header(psf[idmap[0]].fptr);

	if (psf[idmap[0]].mode != Integration::SEARCH)
	{
		BOOST_LOG_TRIVIAL(error)<<"mode is not SEARCH";
		exit(-1);
	}

	if (beam.empty())
	{
		if (strcmp(psf[idmap[0]].primary.ibeam, "") != 0)
		{
			BOOST_LOG_TRIVIAL(info)<<"read beam_id from file";
			beam = psf[idmap[0]].primary.ibeam;
		}
	}
	
	if (source_name.empty())
	{
		if (strcmp(psf[idmap[0]].primary.src_name, "") != 0)
		{
			BOOST_LOG_TRIVIAL(info)<<"read source name from file";
			source_name = psf[idmap[0]].primary.src_name;
		}
	}

	if (telescope.empty())
	{
		if (strcmp(psf[idmap[0]].primary.telesop, "") != 0)
		{
			BOOST_LOG_TRIVIAL(info)<<"read telescope from file";
			telescope = psf[idmap[0]].primary.telesop;
		}
	}

	if (ra.empty())
	{
		if (strcmp(psf[idmap[0]].primary.ra, "") != 0)
		{
			BOOST_LOG_TRIVIAL(info)<<"read ra from file";
			ra = psf[idmap[0]].primary.ra;
		}
	}

	if (dec.empty())
	{
		if (strcmp(psf[idmap[0]].primary.dec, "") != 0)
		{
			BOOST_LOG_TRIVIAL(info)<<"read dec from file";
			dec = psf[idmap[0]].primary.dec;
		}
	}

	psf[idmap[0]].subint.load_integration(psf[idmap[0]].fptr, 0, it);

	it8 = it;
	it8.dtype = Integration::UINT8;
	it8.nbits = 8;
	delete [] (unsigned char *)(it8.data);
	it8.data = new unsigned char [it.nsblk * it.npol * it.nchan];

	nchans = it.nchan;
	tsamp = psf[idmap[0]].subint.tbin;
	nifs = it.npol;
	nsblk = it.nsblk;

	start_mjd = psf[idmap[0]].primary.start_mjd + (it.offs_sub - 0.5 * psf[idmap[0]].subint.nsblk * psf[idmap[0]].subint.tbin);

	frequencies.resize(nchans, 0.);
	std::memcpy(frequencies.data(), it.frequencies, sizeof(double)*nchans);

	psf[idmap[0]].close();
}

size_t PsrfitsReader::read_data(DataBuffer<float> &databuffer, size_t ndump, bool virtual_reading)
{
	assert(databuffer.buffer.size() > 0);

	size_t bcnt1 = 0;

	size_t npsf = psf.size();
	for (size_t idxn=ifile_cur; idxn<npsf; idxn++)
	{
		size_t n = idmap[idxn];

		if (update_file)
		{
			psf[n].open();
			psf[n].primary.load(psf[n].fptr);
			psf[n].load_mode();
			psf[n].subint.load_header(psf[n].fptr);

			ns_psfn = 0;
		}
		update_file = false;

		double zero_off = 0.;
		if (apply_zero_off)
			zero_off = psf[n].subint.zero_off;

		for (size_t s=isubint_cur; s<psf[n].subint.nsubint; s++)
		{
			if (verbose)
			{
				cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
				cerr<<"("<<100.*count/nsamples<<"%)";
			}

			if (!virtual_reading)
			{
				if (update_subint)
				{
					if (apply_scloffs or apply_wts)
						psf[n].subint.load_integration(psf[n].fptr, s, it);
					else
						psf[n].subint.load_integration_data(psf[n].fptr, s, it);

					if (it.dtype != Integration::FLOAT and it.dtype != Integration::UINT8)
						it.to_char(it8);
				}
				update_subint = false;
			}

			for (size_t i=isample_cur; i<it.nsblk; i++)
			{
				if (!virtual_reading)
				{
					if (it.dtype == Integration::UINT8)
					{
						if (!sumif or nifs == 1)
						{
							for (size_t k=0; k<nifs; k++)
							{
								if (apply_wts)
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it.weights[j] * ((((unsigned char *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it.scales[k*nchans+j] + it.offsets[k*nchans+j]);
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it.weights[j] * (((unsigned char *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off);
										}
									}
								}
								else
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = (((unsigned char *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it.scales[k*nchans+j] + it.offsets[k*nchans+j];
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = ((unsigned char *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off;
										}
									}
								}
							}
						}
						else
						{
							if (apply_wts)
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it.weights[j] * ((((unsigned char *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it.scales[0*nchans+j] + it.offsets[0*nchans+j]);
										float yy = it.weights[j] * ((((unsigned char *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it.scales[1*nchans+j] + it.offsets[1*nchans+j]);
										
										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it.weights[j] * (((unsigned char *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off);
										float yy = it.weights[j] * (((unsigned char *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off);

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
							}
							else
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = (((unsigned char *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it.scales[0*nchans+j] + it.offsets[0*nchans+j];
										float yy = (((unsigned char *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it.scales[1*nchans+j] + it.offsets[1*nchans+j];

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = ((unsigned char *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off;
										float yy = ((unsigned char *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off;

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
							}
						}
					}
					else if (it.dtype == Integration::UINT1 or it.dtype == Integration::UINT2 or it.dtype == Integration::UINT4)
					{
						if (!sumif or nifs == 1)
						{
							for (size_t k=0; k<nifs; k++)
							{
								if (apply_wts)
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it8.weights[j] * ((((unsigned char *)(it8.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it8.scales[k*nchans+j] + it8.offsets[k*nchans+j]);
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it8.weights[j] * (((unsigned char *)(it8.data))[i*nifs*nchans+k*nchans+j] - zero_off);
										}
									}
								}
								else
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = (((unsigned char *)(it8.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it8.scales[k*nchans+j] + it8.offsets[k*nchans+j];
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = ((unsigned char *)(it8.data))[i*nifs*nchans+k*nchans+j] - zero_off;
										}
									}
								}
							}
						}
						else
						{
							if (apply_wts)
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it8.weights[j] * ((((unsigned char *)(it8.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it8.scales[0*nchans+j] + it8.offsets[0*nchans+j]);
										float yy = it8.weights[j] * ((((unsigned char *)(it8.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it8.scales[1*nchans+j] + it8.offsets[1*nchans+j]);
										
										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it8.weights[j] * (((unsigned char *)(it8.data))[i*nifs*nchans+0*nchans+j] - zero_off);
										float yy = it8.weights[j] * (((unsigned char *)(it8.data))[i*nifs*nchans+1*nchans+j] - zero_off);

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
							}
							else
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = (((unsigned char *)(it8.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it8.scales[0*nchans+j] + it8.offsets[0*nchans+j];
										float yy = (((unsigned char *)(it8.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it8.scales[1*nchans+j] + it8.offsets[1*nchans+j];

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = ((unsigned char *)(it8.data))[i*nifs*nchans+0*nchans+j] - zero_off;
										float yy = ((unsigned char *)(it8.data))[i*nifs*nchans+1*nchans+j] - zero_off;

										databuffer.buffer[bcnt1*nchans+j] = xx + yy;
									}
								}
							}
						}
					}
					else if (it.dtype == Integration::FLOAT) //for float IQUV data
					{
						if (!sumif or nifs == 1)
						{
							for (size_t k=0; k<nifs; k++)
							{
								if (apply_wts)
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it.weights[j] * ((((float *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it.scales[k*nchans+j] + it.offsets[k*nchans+j]);
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = it.weights[j] * (((float *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off);
										}
									}
								}
								else
								{
									if (apply_scloffs)
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = (((float *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off) * it.scales[k*nchans+j] + it.offsets[k*nchans+j];
										}
									}
									else
									{
										for (size_t j=0; j<nchans; j++)
										{
											databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = ((float *)(it.data))[i*nifs*nchans+k*nchans+j] - zero_off;
										}
									}
								}
							}
						}
						else
						{
							if (apply_wts)
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it.weights[j] * ((((float *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it.scales[0*nchans+j] + it.offsets[0*nchans+j]);
										float yy = it.weights[j] * ((((float *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it.scales[1*nchans+j] + it.offsets[1*nchans+j]);
										
										databuffer.buffer[bcnt1*nchans+j] = xx;// + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = it.weights[j] * (((float *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off);
										float yy = it.weights[j] * (((float *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off);

										databuffer.buffer[bcnt1*nchans+j] = xx;// + yy;
									}
								}
							}
							else
							{
								if (apply_scloffs)
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = (((float *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off) * it.scales[0*nchans+j] + it.offsets[0*nchans+j];
										float yy = (((float *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off) * it.scales[1*nchans+j] + it.offsets[1*nchans+j];

										databuffer.buffer[bcnt1*nchans+j] = xx;// + yy;
									}
								}
								else
								{
									for (size_t j=0; j<nchans; j++)
									{
										float xx = ((float *)(it.data))[i*nifs*nchans+0*nchans+j] - zero_off;
										float yy = ((float *)(it.data))[i*nifs*nchans+1*nchans+j] - zero_off;

										databuffer.buffer[bcnt1*nchans+j] = xx;// + yy;
									}
								}
							}
						}
					}
					else
					{
						BOOST_LOG_TRIVIAL(error)<<"data type is not supported"<<endl;
					}
				}

				bcnt1++;
				count++;
				ntot++;
				ns_psfn++;

				if (count == nsamples - skip_end)
				{
					if (ns_psfn == psf[n].subint.nsamples)
					{
						psf[n].close();
						update_file = true;
						update_subint = true;
					}

					is_end = true;
					if (verbose)
					{
						cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
						cerr<<"("<<100.*count/nsamples<<"%)";
					}
					return bcnt1;
				}

				if (bcnt1 == ndump)
				{
					ifile_cur = idxn;
					isubint_cur = s;
					isample_cur = i+1;

					if (isample_cur == it.nsblk)
					{
						isample_cur = 0;
						isubint_cur++;
						update_subint = true;
					}

					if (ns_psfn == psf[n].subint.nsamples)
					{
						isample_cur = 0;
						isubint_cur = 0;
						ifile_cur++;

						psf[n].close();
						update_file = true;
						update_subint = true;
					}

					return bcnt1;
				}

				if (ns_psfn == psf[n].subint.nsamples)
				{
					goto next;
				}
			}
			isample_cur = 0;
			update_subint = true;
		}
		next:
		isample_cur = 0;
		isubint_cur = 0;
		psf[n].close();
		update_file = true;
		update_subint = true;
	}

	is_end = true;
	if (verbose)
	{
		cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
		cerr<<"("<<100.*count/nsamples<<"%)";
	}
	return bcnt1;
}

size_t PsrfitsReader::read_data(DataBuffer<unsigned char> &databuffer, size_t ndump, bool virtual_reading)
{
	assert(databuffer.buffer.size() > 0);

	size_t bcnt1 = 0;

	size_t npsf = psf.size();
	for (size_t idxn=ifile_cur; idxn<npsf; idxn++)
	{
		size_t n = idmap[idxn];

		if (update_file)
		{
			psf[n].open();
			psf[n].primary.load(psf[n].fptr);
			psf[n].load_mode();
			psf[n].subint.load_header(psf[n].fptr);

			ns_psfn = 0;
		}
		update_file = false;

		double zero_off = 0.;
		if (apply_zero_off)
			zero_off = psf[n].subint.zero_off;

		for (size_t s=isubint_cur; s<psf[n].subint.nsubint; s++)
		{
			if (verbose)
			{
				cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
				cerr<<"("<<100.*count/nsamples<<"%)";
			}

			if (!virtual_reading)
			{
				if (update_subint)
				{
					psf[n].subint.load_integration_data(psf[n].fptr, s, it);
				}
				update_subint = false;
			}

			for (size_t i=isample_cur; i<it.nsblk; i++)
			{
				if (!virtual_reading)
				{
					if (it.dtype == Integration::UINT8)
					{
						if (!sumif or nifs == 1)
						{
							for (size_t k=0; k<nifs; k++)
							{
								for (size_t j=0; j<nchans; j++)
								{
									databuffer.buffer[bcnt1*nifs*nchans+k*nchans+j] = ((unsigned char *)(it.data))[i*nifs*nchans+k*nchans+j];
								}
							}
						}
						else
						{
							for (size_t j=0; j<nchans; j++)
							{
								unsigned char xx = ((unsigned char *)(it.data))[i*nifs*nchans+0*nchans+j];
								unsigned char yy = ((unsigned char *)(it.data))[i*nifs*nchans+1*nchans+j];

								databuffer.buffer[bcnt1*nchans+j] = (xx + yy) / 2;
							}
						}
					}
					else
					{
						BOOST_LOG_TRIVIAL(error)<<"data type is not supported"<<endl;
					}
				}

				bcnt1++;
				count++;
				ntot++;
				ns_psfn++;

				if (count == nsamples - skip_end)
				{
					if (ns_psfn == psf[n].subint.nsamples)
					{
						psf[n].close();
						update_file = true;
						update_subint = true;
					}

					is_end = true;
					if (verbose)
					{
						cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
						cerr<<"("<<100.*count/nsamples<<"%)";
					}
					return bcnt1;
				}

				if (bcnt1 == ndump)
				{
					ifile_cur = idxn;
					isubint_cur = s;
					isample_cur = i+1;

					if (isample_cur == it.nsblk)
					{
						isample_cur = 0;
						isubint_cur++;
						update_subint = true;
					}

					if (ns_psfn == psf[n].subint.nsamples)
					{
						isample_cur = 0;
						isubint_cur = 0;
						ifile_cur++;

						psf[n].close();
						update_file = true;
						update_subint = true;
					}

					return bcnt1;
				}

				if (ns_psfn == psf[n].subint.nsamples)
				{
					goto next;
				}
			}
			isample_cur = 0;
			update_subint = true;
		}
		next:
		isample_cur = 0;
		isubint_cur = 0;
		psf[n].close();
		update_file = true;
		update_subint = true;
	}

	is_end = true;
	if (verbose)
	{
		cerr<<"\r\rfinish "<<setprecision(2)<<fixed<<tsamp*count<<" seconds ";
		cerr<<"("<<100.*count/nsamples<<"%)";
	}
	return bcnt1;
}

void PsrfitsReader::get_filterbank_template(Filterbank &filtem)
{
	filtem.use_frequence_table = false;
	filtem.data_type = 1;
	strcpy(filtem.rawdatafile, psf[idmap[0]].filename.substr(psf[idmap[0]].filename.find_last_of("/\\") + 1).c_str());
	filtem.tstart =start_mjd.to_day();
	filtem.tsamp = tsamp;
	filtem.nifs = nifs;
	if (sumif) filtem.nifs = 1;
	filtem.nchans = nchans;
	
	get_fch1_foff(filtem.fch1, filtem.foff);

	if (filtem.frequency_table != NULL) delete [] filtem.frequency_table;
	filtem.frequency_table = new double [16320];
	memcpy(filtem.frequency_table, frequencies.data(), sizeof(double)*nchans);
}