#include "AssemblerUtils.hpp"

namespace poly_fem
{
	AssemblerUtils &AssemblerUtils::instance()
	{
		static AssemblerUtils instance;

		return instance;
	}


	AssemblerUtils::AssemblerUtils()
	{
		scalar_assemblers_.push_back("Laplacian");
		scalar_assemblers_.push_back("Helmholtz");

		tensor_assemblers_.push_back("LinearElasticity");
		tensor_assemblers_.push_back("HookeLinearElasticity");
		tensor_assemblers_.push_back("SaintVenant");
	}

	bool AssemblerUtils::is_linear(const std::string &assembler) const
	{
		return assembler != "SaintVenant";
	}

	void AssemblerUtils::assemble_scalar_problem(const std::string &assembler,
		const bool is_volume,
		const int n_basis,
		const std::vector< ElementBases > &bases,
		const std::vector< ElementBases > &gbases,
		Eigen::SparseMatrix<double> &stiffness) const
	{
		if(assembler == "Helmholtz")
			helmholtz_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		else if(assembler == "Laplacian")
			laplacian_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		else
		{
			std::cerr<<"[Warning] "<<assembler<<" not found, fallback to default"<<std::endl;
			assert(false);
			laplacian_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		}
	}

	void AssemblerUtils::assemble_tensor_problem(const std::string &assembler,
		const bool is_volume,
		const int n_basis,
		const std::vector< ElementBases > &bases,
		const std::vector< ElementBases > &gbases,
		Eigen::SparseMatrix<double> &stiffness) const
	{
		if(assembler == "HookeLinearElasticity")
			hooke_linear_elasticity_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		else if(assembler == "SaintVenant")
			return;
		else if(assembler == "LinearElasticity")
			linear_elasticity_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		else
		{
			std::cerr<<"[Warning] "<<assembler<<" not found, fallback to default"<<std::endl;
			assert(false);
			linear_elasticity_.assemble(is_volume, n_basis, bases, gbases, stiffness);
		}
	}




	double AssemblerUtils::assemble_tensor_energy(const std::string &assembler,
		const bool is_volume,
		const std::vector< ElementBases > &bases,
		const std::vector< ElementBases > &gbases,
		const Eigen::MatrixXd &displacement) const
	{
		if(assembler != "SaintVenant") return 0;

		return saint_venant_elasticity_.compute_energy(is_volume, bases, gbases, displacement);
	}

	void AssemblerUtils::assemble_tensor_energy_gradient(const std::string &assembler,
		const bool is_volume,
		const int n_basis,
		const std::vector< ElementBases > &bases,
		const std::vector< ElementBases > &gbases,
		const Eigen::MatrixXd &displacement,
		Eigen::MatrixXd &grad) const
	{
		if(assembler != "SaintVenant") return;

		saint_venant_elasticity_.assemble(is_volume, n_basis, bases, gbases, displacement, grad);
	}

	void AssemblerUtils::assemble_tensor_energy_hessian(const std::string &assembler,
		const bool is_volume,
		const int n_basis,
		const std::vector< ElementBases > &bases,
		const std::vector< ElementBases > &gbases,
		const Eigen::MatrixXd &displacement,
		Eigen::SparseMatrix<double> &hessian) const
	{
		if(assembler != "SaintVenant") return;

		saint_venant_elasticity_.assemble_grad(is_volume, n_basis, bases, gbases, displacement, hessian);
	}


	void AssemblerUtils::compute_scalar_value(const std::string &assembler,
		const ElementBases &bs,
		const Eigen::MatrixXd &local_pts,
		const Eigen::MatrixXd &fun,
		Eigen::MatrixXd &result) const
	{
		if(assembler == "Laplacian" || assembler == "Helmholtz")
			return;
		else if(assembler == "HookeLinearElasticity")
			hooke_linear_elasticity_.local_assembler().compute_von_mises_stresses(bs, local_pts, fun, result);
		else if(assembler == "SaintVenant")
			saint_venant_elasticity_.local_assembler().compute_von_mises_stresses(bs, local_pts, fun, result);
		else if(assembler == "LinearElasticity")
			linear_elasticity_.local_assembler().compute_von_mises_stresses(bs, local_pts, fun, result);
		else
		{
			std::cerr<<"[Warning] "<<assembler<<" not found, fallback to default"<<std::endl;
			assert(false);
			linear_elasticity_.local_assembler().compute_von_mises_stresses(bs, local_pts, fun, result);
		}
	}


	VectorNd AssemblerUtils::compute_rhs(const std::string &assembler, const AutodiffHessianPt &pt) const
	{
		if(assembler == "Laplacian")
			return laplacian_.local_assembler().compute_rhs(pt);
		else if(assembler == "Helmholtz")
			return helmholtz_.local_assembler().compute_rhs(pt);
		else if(assembler == "LinearElasticity")
			return linear_elasticity_.local_assembler().compute_rhs(pt);
		else if(assembler == "HookeLinearElasticity")
			return hooke_linear_elasticity_.local_assembler().compute_rhs(pt);
		// else if(assembler == "SaintVenant")
			//return saint_venant_elasticity_.local_assembler().compute_rhs(pt);
		else
		{
			std::cerr<<"[Warning] "<<assembler<<" not found, fallback to default"<<std::endl;

			assert(false);
			return laplacian_.local_assembler().compute_rhs(pt);
		}

	}



	void AssemblerUtils::set_parameters(const json &params)
	{
		laplacian_.local_assembler().set_parameters(params);
		helmholtz_.local_assembler().set_parameters(params);

		linear_elasticity_.local_assembler().set_parameters(params);
		hooke_linear_elasticity_.local_assembler().set_parameters(params);
		saint_venant_elasticity_.local_assembler().set_parameters(params);
	}

}