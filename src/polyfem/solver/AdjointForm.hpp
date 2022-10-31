#pragma once

#include <polyfem/State.hpp>

namespace polyfem::solver
{
	class AdjointForm
	{
	public:
		AdjointForm(const std::string &type)
		{
			// TODO: build IntegrableFunctional j based on type
		}

		double value(
			const State &state,
			const IntegrableFunctional &j,
			const std::set<int> &interested_ids, // either body id or surface id
			const bool is_volume_integral,
			const std::string &transient_integral_type = "");

		void gradient(
			const State &state,
			const IntegrableFunctional &j,
			const std::string &param,
			Eigen::VectorXd &grad,
			const std::set<int> &interested_ids, // either body id or surface id
			const bool is_volume_integral,
			const std::string &transient_integral_type = "");

	protected:

		static double integrate_objective(
			const State &state, 
			const IntegrableFunctional &j, 
			const Eigen::MatrixXd &solution,
			const std::set<int> &interested_ids, // either body id or surface id
			const bool is_volume_integral,
			const int cur_step = 0);
		static void dJ_du_step(
			const State &state,
			const IntegrableFunctional &j, 
			const Eigen::MatrixXd &solution,
			const std::set<int> &interested_ids,
			const bool is_volume_integral,
			const int cur_step,
			Eigen::VectorXd &term);
		static double integrate_objective_transient(
			const State &state, 
			const IntegrableFunctional &j,
			const std::set<int> &interested_ids,
			const bool is_volume_integral,
			const std::string &transient_integral_type);
		static void compute_shape_derivative_functional_term(
			const State &state,
			const Eigen::MatrixXd &solution, 
			const IntegrableFunctional &j, 
			const std::set<int> &interested_ids, // either body id or surface id 
			const bool is_volume_integral,
			Eigen::VectorXd &term, 
			const int cur_time_step);
		static void dJ_shape_static(
			const State &state,
			const Eigen::MatrixXd &sol,
			const Eigen::MatrixXd &adjoint,
			const IntegrableFunctional &j,
			const std::set<int> &interested_ids,
			const bool is_volume_integral,
			Eigen::VectorXd &one_form);
		static void dJ_shape_transient(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			const IntegrableFunctional &j,
			const std::set<int> &interested_ids,
			const bool is_volume_integral,
			const std::string &transient_integral_type,
			Eigen::VectorXd &one_form);
		static void dJ_material_static(
			const State &state,
			const Eigen::MatrixXd &sol,
			const Eigen::MatrixXd &adjoint,
			Eigen::VectorXd &one_form);
		static void dJ_material_transient(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			Eigen::VectorXd &one_form);
		static void dJ_friction_transient(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			Eigen::VectorXd &one_form);
		static void dJ_damping_transient(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			Eigen::VectorXd &one_form);
		static void dJ_initial_condition(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			Eigen::VectorXd &one_form);
		static void dJ_dirichlet_transient(
			const State &state,
			const std::vector<Eigen::MatrixXd> &adjoint_nu,
			const std::vector<Eigen::MatrixXd> &adjoint_p,
			Eigen::VectorXd &one_form);
	};
} // namespace polyfem::solver
