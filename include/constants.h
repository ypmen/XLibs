/**
 * @author Yunpeng Men
 * @email ypmen@pku.edu.cn
 * @create date 2020-05-19 22:52:10
 * @modify date 2020-05-19 22:52:10
 * @desc [description]
 */

#ifndef CONSTANTS
#define CONSTANTS

#define CONST_C 299792458 /*m/s*/

inline double fdot2acc(double fdot, double f)
{
	return -fdot/f*CONST_C;
}

inline double acc2fdot(double acc, double f)
{
	return -f*acc/CONST_C;
}

#define CONST_DM (1./2.41e-4)

inline double dispersion_delay(double dm, double fh, double fl)
{
	return CONST_DM*dm*(1./(fl*fl)-1./(fh*fh));
}

#endif /* CONSTANTS */
