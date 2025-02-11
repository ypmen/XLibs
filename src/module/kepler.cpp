#include "kepler.h"
#include <cmath>

namespace Pulsar {

	Kepler::Kepler() : f0(0.0), Pb(0.0), a1(0.0), phi_T0(0.0), om(0.0), ecc(0.0) {}

	Kepler::Kepler(double f0_in, double Pb_in, double a1_in, double phi_T0_in, double om_in, double ecc_in) :
		f0(f0_in), Pb(Pb_in), a1(a1_in), phi_T0(phi_T0_in), om(om_in), ecc(ecc_in) {}

	Kepler::Kepler(std::vector<double> &params) :
		f0(params[0]), Pb(params[1]), a1(params[2]), phi_T0(params[3]), om(params[4]), ecc(params[5]) {}

	Kepler::~Kepler() {}

	double Kepler::get_fphase(double T, double Tref)
	{		
		double roemer_delay = get_roemer(T);

		double phi = f0 * ((T - Tref) * 86400. + roemer_delay);
		
		phi -= floor(phi);

		return phi;
	}

	double Kepler::get_ffold(double T)
	{
		double sqrt_1_ee = std::sqrt(1. - ecc * ecc);

		double E = get_ecc_anomaly(T);

		double sin_E = std::sin(E);
		double cos_E = std::cos(E);

		double sin_omega = std::sin(om);
		double cos_omega = std::cos(om);

		double temp = 2 * M_PI / (Pb * 86400. * (1. - ecc * cos_E));

		double doppler_factor1 = a1 * (-sin_omega * sin_E + sqrt_1_ee * cos_omega * cos_E) * temp;

		return f0 * (1. + doppler_factor1);
	}

	double Kepler::get_fdfold(double T)
	{
		double sqrt_1_ee = std::sqrt(1. - ecc * ecc);

		double E = get_ecc_anomaly(T);

		double sin_E = std::sin(E);
		double cos_E = std::cos(E);

		double sin_omega = std::sin(om);
		double cos_omega = std::cos(om);

		double dE_dT = 2 * M_PI / (Pb * 86400. * (1. - ecc * cos_E));

		double dR_dE = a1 * (-sin_omega * sin_E + sqrt_1_ee * cos_omega * cos_E);

		double d2R_dE2 = a1 * (-cos_E * sin_omega - sqrt_1_ee * sin_E * cos_omega);

		double d2E_dT2 = - dE_dT * dE_dT * ecc * sin_E / (1. - ecc * cos_E);

		double doppler_factor2 = d2R_dE2 * dE_dT * dE_dT + dR_dE * d2E_dT2;

		return f0 * doppler_factor2;
	}

	double Kepler::get_roemer(double T)
	{
		double sqrt_1_ee = std::sqrt(1. - ecc * ecc);

		double E = get_ecc_anomaly(T);

		double sin_E = std::sin(E);
		double cos_E = std::cos(E);

		double sin_omega = std::sin(om);
		double cos_omega = std::cos(om);

		double roemer_delay = a1 * ((cos_E - ecc) * sin_omega + sqrt_1_ee * sin_E * cos_omega);

		return roemer_delay;
	}

	double Kepler::get_ecc_anomaly(double T)
	{
		double orbit = T / Pb - phi_T0;
		double norbit = std::floor(orbit);
		double forbit = orbit - norbit;

		// calculate eccentric anomaly
		double phase = 2. * M_PI * forbit;
		double E = phase + ecc * std::sin(phase) / std::sqrt(1. - 2. * ecc * std::cos(phase) + ecc * ecc);
		double dE = 1.;
		size_t nstep = 0;
		while (std::abs(dE) > 1e-14 && nstep++ < 8)
		{
			dE = (phase - (E - ecc * std::sin(E))) / (1. - ecc * std::cos(E));
			E += dE;
		}

		return E;
	}

} // namespace Pulsar
