#pragma once

#include <polyfem/assembler/LinearElasticity.hpp>
#include <polyfem/assembler/HookeLinearElasticity.hpp>
#include <polyfem/assembler/SaintVenantElasticity.hpp>
#include <polyfem/assembler/NeoHookeanElasticity.hpp>
#include <polyfem/assembler/MooneyRivlinElasticity.hpp>
#include <polyfem/assembler/MooneyRivlin3ParamElasticity.hpp>
#include <polyfem/assembler/OgdenElasticity.hpp>
#include <polyfem/basis/Basis.hpp>

namespace polyfem::assembler
{
	class MultiModel : public NLAssembler, ElasticityAssembler
	{
	public:
		using NLAssembler::assemble_energy;
		using NLAssembler::assemble_gradient;
		using NLAssembler::assemble_hessian;

		// compute elastic energy
		double compute_energy(const NonLinearAssemblerData &data) const override;
		// neccessary for mixing linear model with non-linear collision response
		Eigen::MatrixXd assemble_hessian(const NonLinearAssemblerData &data) const override;
		// compute gradient of elastic energy, as assembler
		Eigen::VectorXd assemble_gradient(const NonLinearAssemblerData &data) const override;

		// uses autodiff to compute the rhs for a fabbricated solution
		// uses autogenerated code to compute div(sigma)
		// pt is the evaluation of the solution at a point
		VectorNd compute_rhs(const AutodiffHessianPt &pt) const override;
		void set_size(const int size) override;

		// inialize material parameter
		void add_multimaterial(const int index, const json &params, const Units &units) override;

		// initialized multi models
		inline void init_multimodels(const std::vector<std::string> &mats) { multi_material_models_ = mats; }

		std::string name() const override { return "MultiModels"; }
		bool allow_inversion() const override { return true; }
		std::map<std::string, ParamFunc> parameters() const override;

	protected:
		void assign_stress_tensor(const OutputData &data,
								  const int all_size,
								  const ElasticityTensorType &type,
								  Eigen::MatrixXd &all,
								  const std::function<Eigen::MatrixXd(const Eigen::MatrixXd &)> &fun) const override;

	private:
		std::vector<std::string> multi_material_models_;

		SaintVenantElasticity saint_venant_;
		NeoHookeanElasticity neo_hookean_;
		LinearElasticity linear_elasticity_;
		HookeLinearElasticity hooke_;
		MooneyRivlinElasticity mooney_rivlin_elasticity_;
		MooneyRivlin3ParamElasticity mooney_rivlin_3_param_elasticity_;
		UnconstrainedOgdenElasticity unconstrained_ogden_elasticity_;
		IncompressibleOgdenElasticity incompressible_ogden_elasticity_;
	};
} // namespace polyfem::assembler
