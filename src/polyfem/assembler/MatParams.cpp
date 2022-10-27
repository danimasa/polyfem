#include "MatParams.hpp"

namespace polyfem::assembler
{
	namespace
	{
		double convert_to_lambda(const bool is_volume, const double E, const double nu)
		{
			if (is_volume)
				return (E * nu) / ((1.0 + nu) * (1.0 - 2.0 * nu));

			return (nu * E) / (1.0 - nu * nu);
		}

		double convert_to_mu(const double E, const double nu)
		{
			return E / (2.0 * (1.0 + nu));
		}
	} // namespace

	GenericMatParam::GenericMatParam(const std::string &param_name)
		: param_name_(param_name)
	{
	}

	void GenericMatParam::add_multimaterial(const int index, const json &params)
	{
		for (int i = param_.size(); i <= index; ++i)
		{
			param_.emplace_back();
		}

		if (params.count(param_name_))
			param_[index].init(params[param_name_]);
	}

	double GenericMatParam::operator()(const RowVectorNd &p, double t, int index) const
	{
		const double x = p(0);
		const double y = p(1);
		const double z = p.size() == 3 ? p(2) : 0;

		return (*this)(x, y, z, t, index);
	}

	double GenericMatParam::operator()(double x, double y, double z, double t, int index) const
	{
		assert(param_.size() == 1 || index < param_.size());

		const auto &tmp_param = param_.size() == 1 ? param_[0] : param_[index];

		return tmp_param(x, y, z, t, index);
	}

	void ElasticityTensor::resize(const int size)
	{
		if (size == 2)
			stifness_tensor_.resize(3, 3);
		else
			stifness_tensor_.resize(6, 6);

		stifness_tensor_.setZero();

		size_ = size;
	}

	double ElasticityTensor::operator()(int i, int j) const
	{
		if (j < i)
		{
			std::swap(i, j);
		}

		assert(j >= i);
		return stifness_tensor_(i, j);
	}

	double &ElasticityTensor::operator()(int i, int j)
	{
		if (j < i)
		{
			std::swap(i, j);
		}

		assert(j >= i);
		return stifness_tensor_(i, j);
	}

	void ElasticityTensor::set_from_entries(const std::vector<double> &entries)
	{
		if (size_ == 2)
		{
			if (entries.size() == 4)
			{
				set_orthotropic(
					entries[0],
					entries[1],
					entries[2],
					entries[3]);

				return;
			}

			assert(entries.size() >= 6);

			(*this)(0, 0) = entries[0];
			(*this)(0, 1) = entries[1];
			(*this)(0, 2) = entries[2];

			(*this)(1, 1) = entries[3];
			(*this)(1, 2) = entries[4];

			(*this)(2, 2) = entries[5];
		}
		else
		{
			if (entries.size() == 9)
			{
				set_orthotropic(
					entries[0],
					entries[1],
					entries[2],
					entries[3],
					entries[4],
					entries[5],
					entries[6],
					entries[7],
					entries[8]);

				return;
			}
			assert(entries.size() >= 21);

			(*this)(0, 0) = entries[0];
			(*this)(0, 1) = entries[1];
			(*this)(0, 2) = entries[2];
			(*this)(0, 3) = entries[3];
			(*this)(0, 4) = entries[4];
			(*this)(0, 5) = entries[5];

			(*this)(1, 1) = entries[6];
			(*this)(1, 2) = entries[7];
			(*this)(1, 3) = entries[8];
			(*this)(1, 4) = entries[9];
			(*this)(1, 5) = entries[10];

			(*this)(2, 2) = entries[11];
			(*this)(2, 3) = entries[12];
			(*this)(2, 4) = entries[13];
			(*this)(2, 5) = entries[14];

			(*this)(3, 3) = entries[15];
			(*this)(3, 4) = entries[16];
			(*this)(3, 5) = entries[17];

			(*this)(4, 4) = entries[18];
			(*this)(4, 5) = entries[19];

			(*this)(5, 5) = entries[20];
		}
	}

	void ElasticityTensor::set_from_lambda_mu(const double lambda, const double mu)
	{
		if (size_ == 2)
		{
			(*this)(0, 0) = 2 * mu + lambda;
			(*this)(0, 1) = lambda;
			(*this)(0, 2) = 0;

			(*this)(1, 1) = 2 * mu + lambda;
			(*this)(1, 2) = 0;

			(*this)(2, 2) = mu;
		}
		else
		{
			(*this)(0, 0) = 2 * mu + lambda;
			(*this)(0, 1) = lambda;
			(*this)(0, 2) = lambda;
			(*this)(0, 3) = 0;
			(*this)(0, 4) = 0;
			(*this)(0, 5) = 0;

			(*this)(1, 1) = 2 * mu + lambda;
			(*this)(1, 2) = lambda;
			(*this)(1, 3) = 0;
			(*this)(1, 4) = 0;
			(*this)(1, 5) = 0;

			(*this)(2, 2) = 2 * mu + lambda;
			(*this)(2, 3) = 0;
			(*this)(2, 4) = 0;
			(*this)(2, 5) = 0;

			(*this)(3, 3) = mu;
			(*this)(3, 4) = 0;
			(*this)(3, 5) = 0;

			(*this)(4, 4) = mu;
			(*this)(4, 5) = 0;

			(*this)(5, 5) = mu;
		}
	}

	void ElasticityTensor::set_from_young_poisson(const double young, const double nu)
	{
		if (size_ == 2)
		{
			stifness_tensor_ << 1.0, nu, 0.0,
				nu, 1.0, 0.0,
				0.0, 0.0, (1.0 - nu) / 2.0;
			stifness_tensor_ *= young / (1.0 - nu * nu);
		}
		else
		{
			assert(size_ == 3);
			const double v = nu;
			stifness_tensor_ << 1. - v, v, v, 0, 0, 0,
				v, 1. - v, v, 0, 0, 0,
				v, v, 1. - v, 0, 0, 0,
				0, 0, 0, (1. - 2. * v) / 2., 0, 0,
				0, 0, 0, 0, (1. - 2. * v) / 2., 0,
				0, 0, 0, 0, 0, (1. - 2. * v) / 2.;
			stifness_tensor_ *= young / ((1. + v) * (1. - 2. * v));
		}
	}

	void ElasticityTensor::set_orthotropic(
		double Ex, double Ey, double Ez,
		double nuYX, double nuZX, double nuZY,
		double muYZ, double muZX, double muXY)
	{
		// copied from Julian
		assert(size_ == 3);
		// Note: this isn't the flattened compliance tensor! Rather, it is the
		// matrix inverse of the flattened elasticity tensor. See the tensor
		// flattening writeup.
		stifness_tensor_ << 1.0 / Ex, -nuYX / Ey, -nuZX / Ez, 0.0, 0.0, 0.0,
			0.0, 1.0 / Ey, -nuZY / Ez, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0 / Ez, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0 / muYZ, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 1.0 / muZX, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / muXY;
	}

	void ElasticityTensor::set_orthotropic(double Ex, double Ey, double nuYX, double muXY)
	{
		// copied from Julian
		assert(size_ == 2);
		// Note: this isn't the flattened compliance tensor! Rather, it is the
		// matrix inverse of the flattened elasticity tensor.
		stifness_tensor_ << 1.0 / Ex, -nuYX / Ey, 0.0,
			0.0, 1.0 / Ey, 0.0,
			0.0, 0.0, 1.0 / muXY;
	}

	template <int DIM>
	double ElasticityTensor::compute_stress(const std::array<double, DIM> &strain, const int j) const
	{
		double res = 0;

		for (int k = 0; k < DIM; ++k)
			res += (*this)(j, k) * strain[k];

		return res;
	}

	LameParameters::LameParameters()
	{
		lambda_or_E_.emplace_back();
		lambda_or_E_.back().init(1.0);

		mu_or_nu_.emplace_back();
		mu_or_nu_.back().init(1.0);
		size_ = -1;
		is_lambda_mu_ = true;
	}

	void LameParameters::lambda_mu(double px, double py, double pz, double x, double y, double z, int el_id, double &lambda, double &mu, bool has_density) const
	{
		assert(lambda_or_E_.size() == 1 || el_id < lambda_or_E_.size());
		assert(mu_or_nu_.size() == 1 || el_id < mu_or_nu_.size());
		assert(size_ == 2 || size_ == 3);

		const auto &tmp1 = lambda_or_E_.size() == 1 ? lambda_or_E_[0] : lambda_or_E_[el_id];
		const auto &tmp2 = mu_or_nu_.size() == 1 ? mu_or_nu_[0] : mu_or_nu_[el_id];

		double llambda = tmp1(x, y, z, 0, el_id);
		double mmu = tmp2(x, y, z, 0, el_id);

		if (!is_lambda_mu_)
		{
			lambda = convert_to_lambda(size_ == 3, llambda, mmu);
			mu = convert_to_mu(llambda, mmu);
		}
		else
		{
			lambda = llambda;
			mu = mmu;
		}

		if (lambda_mat_.size() > el_id && mu_mat_.size() > el_id)
		{
			lambda = lambda_mat_(el_id);
			mu = mu_mat_(el_id);
		}

		if (has_density && el_id < density_mat_.size())
		{
			lambda *= pow(density(el_id), density_power_);
			mu *= pow(density(el_id), density_power_);
		}
		
		assert(!std::isnan(lambda));
		assert(!std::isnan(mu));
		assert(!std::isinf(lambda));
		assert(!std::isinf(mu));
	}

	void LameParameters::add_multimaterial(const int index, const json &params, const bool is_volume)
	{
		const int size = is_volume ? 3 : 2;
		assert(size_ == -1 || size == size_);
		size_ = size;

		for (int i = lambda_or_E_.size(); i <= index; ++i)
		{
			lambda_or_E_.emplace_back();
			mu_or_nu_.emplace_back();
		}

		if (params.count("young"))
		{
			set_e_nu(index, params["young"], params["nu"]);
		}
		else if (params.count("E"))
		{
			set_e_nu(index, params["E"], params["nu"]);
		}
		else if (params.count("lambda"))
		{
			lambda_or_E_[index].init(params["lambda"]);
			mu_or_nu_[index].init(params["mu"]);
			is_lambda_mu_ = true;
		}
	}

	void LameParameters::set_e_nu(const int index, const json &E, const json &nu)
	{
		// TODO: conversion is always called
		is_lambda_mu_ = false;
		lambda_or_E_[index].init(E);
		mu_or_nu_[index].init(nu);
	}

	Density::Density()
	{
		rho_.emplace_back();
		rho_.back().init(1.0);
	}

	double Density::operator()(double px, double py, double pz, double x, double y, double z, int el_id) const
	{
		assert(rho_.size() == 1 || el_id < rho_.size());

		const auto &tmp = rho_.size() == 1 ? rho_[0] : rho_[el_id];
		const double res = tmp(x, y, z, 0, el_id);
		assert(!std::isnan(res));
		assert(!std::isinf(res));
		return res;
	}

	void Density::add_multimaterial(const int index, const json &params)
	{
		for (int i = rho_.size(); i <= index; ++i)
		{
			rho_.emplace_back();
		}

		if (params.count("rho"))
		{
			rho_[index].init(params["rho"]);
		}
		else if (params.count("density"))
		{
			rho_[index].init(params["density"]);
		}
	}

	// template instantiation
	template double ElasticityTensor::compute_stress<3>(const std::array<double, 3> &strain, const int j) const;
	template double ElasticityTensor::compute_stress<6>(const std::array<double, 6> &strain, const int j) const;

} // namespace polyfem::assembler