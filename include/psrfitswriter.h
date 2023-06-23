/**
 * @author Yunpeng Men
 * @email ypmen@pku.edu.cn
 * @create date 2020-05-15 20:31:13
 * @modify date 2020-05-15 20:31:13
 * @desc [description]
 */

#ifndef PSRFITSWRITER
#define PSRFITSWRITER

#include <string.h>

#include "databuffer.h"
#include "dedisperse.h"
#include "psrfits.h"
#include "integration.h"

using namespace std;

template <typename T>
class PsrfitsWriter : public DataBuffer<T>
{
public:
    PsrfitsWriter();
    ~PsrfitsWriter();
    void prepare(DataBuffer<float> &databuffer);
    DataBuffer<T> * run(DataBuffer<float> &databuffer);
    DataBuffer<T> * get(){return this;}
    void close()
    {
        if (fits.fptr != NULL)
        {
            fits.close();
        }
    }
public:
    string rootname;
    string template_file;
    Integration::Mode mode;
    Integration::DataType dtype;
    MJD start_mjd;
    float gain;
    float offs;
    float rms;
    long int nsubint_per_file;
    long int nsamp_per_subint;
    int ibeam;
    int npol;
public:
    string src_name;
    string ra;
    string dec;
    string telescope;
	string obs_mode;
private:
    long int isubint;
    long int ifile;
    long int ichunk;
    Integration it;
    Psrfits fits;
};

#endif /* PSRFITSWRITER */
