#ifndef KEPLER_H
#define KEPLER_H

#include <vector>

namespace Pulsar {
	class Kepler {
	public:
		Kepler();
		Kepler(double f0, double Pb, double a1, double T0, double om, double ecc);
		Kepler(std::vector<double> &params);
		~Kepler();

		double get_fphase(double T, double Tref);
		double get_ffold(double T);
		double get_fdfold(double T);

	public:
		double get_Tp(double T);
		double get_roemer(double T);
		double get_ecc_anomaly(double T);

	public:
		double f0;
		double Pb;
		double a1;
		double T0;
		double om;
		double ecc;
	};
}

#endif /* KEPLER_H */
