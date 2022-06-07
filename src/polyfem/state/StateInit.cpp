#include <polyfem/State.hpp>

#include <polyfem/autogen/auto_p_bases.hpp>
#include <polyfem/autogen/auto_q_bases.hpp>

#include <polyfem/utils/Logger.hpp>
#include <polyfem/problem/KernelProblem.hpp>
#include <polyfem/utils/par_for.hpp>

#include <polysolve/LinearSolver.hpp>

#include <polyfem/utils/JSONUtils.hpp>

#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <ipc/utils/logger.hpp>

namespace polyfem
{
	using namespace problem;
	using namespace utils;

	namespace
	{
		class GeoLoggerForward : public GEO::LoggerClient
		{
			std::shared_ptr<spdlog::logger> logger_;

		public:
			template <typename T>
			GeoLoggerForward(T logger) : logger_(logger) {}

		private:
			std::string truncate(const std::string &msg)
			{
				static size_t prefix_len = GEO::CmdLine::ui_feature(" ", false).size();
				return msg.substr(prefix_len, msg.size() - 1 - prefix_len);
			}

		protected:
			void div(const std::string &title) override
			{
				logger_->trace(title.substr(0, title.size() - 1));
			}

			void out(const std::string &str) override
			{
				logger_->info(truncate(str));
			}

			void warn(const std::string &str) override
			{
				logger_->warn(truncate(str));
			}

			void err(const std::string &str) override
			{
				logger_->error(truncate(str));
			}

			void status(const std::string &str) override
			{
				// Errors and warnings are also dispatched as status by geogram, but without
				// the "feature" header. We thus forward them as trace, to avoid duplicated
				// logger info...
				logger_->trace(str.substr(0, str.size() - 1));
			}
		};
	} // namespace

	State::State(const unsigned int max_threads)
	{
		using namespace polysolve;
#ifndef WIN32
		setenv("GEO_NO_SIGNAL_HANDLER", "1", 1);
#endif

		GEO::initialize();
		const unsigned int num_threads = std::max(1u, std::min(max_threads, std::thread::hardware_concurrency()));
		NThread::get().num_threads = num_threads;
#ifdef POLYFEM_WITH_TBB
		thread_limiter = std::make_shared<tbb::global_control>(tbb::global_control::max_allowed_parallelism, num_threads);
#endif

		// Import standard command line arguments, and custom ones
		GEO::CmdLine::import_arg_group("standard");
		GEO::CmdLine::import_arg_group("pre");
		GEO::CmdLine::import_arg_group("algo");

		problem = ProblemFactory::factory().get_problem("Linear");

		this->args = R"({
						"defaults": "",
						"root_path": "",

						"geometries": null,

					    "space": {
        					"discr_order": 1,
        					"pressure_discr_order": 1,

        					"discr_orders_path": "",
        					"bodies_discr_order": {},

        					"use_p_ref": false,

        					"advanced": {
            					"discr_order_max": 4,

								"serendipity": false,
								"isoparametric": false,
								"use_spline": false,

								"bc_method": "lsq",

								"n_boundary_samples": -1, // Fix in code
								"quadrature_order": -1,

								"poly_bases": "MFSHarmonic",
								"integral_constraints": 2,
								"n_harmonic_samples": 10,
								"force_no_ref_for_harmonic": false,

								"B": 3,
								"h1_formula": false,

								"count_flipped_els": true // always enabled
        					}
    					},

    					"time": {
        					"t0": 0,
        					"tend": -1,
        					"dt": -1,
        					"time_steps": 10,

        					"integrator": "ImplicitEuler",
							"newmark": {
								"gamma": 0.5,
								"beta": 0.25
							},
							"BDF": {
								"steps": 1
							}
						},

						"contact": {
							"dhat": 1e-3,
							"dhat_percentage": 0.8,
							"epsv": 1e-3
						},

						"solver": {
							"linear": {
								"solver": "",
								"precond": "",

								"max_iter" : 1000,
								"conv_tol" : 1e-10,
								"tolerance" : 1e-10,

								"advanced": {}
							},

							"nonlinear": {
								"solver" : "newton",
								"f_delta" : 1e-10,
								"grad_norm" : 1e-8,
								"max_iterations" : 1000,
								"use_grad_norm" : true,
								"relative_gradient" : false,

								"line_search": {
									"method" : "backtracking",
									"use_grad_norm_tol" : 1e-4
								}
							},

							"augmented_lagrangian" : {
								"initial_weight" : 1e6,
								"max_weight" : 1e11,

								"force" : false
							},

							"contact": {
								"CCD" : {
									"broad_phase" : "hash_grid",
									"tolerance" : 1e-6,
									"max_iterations" : 1e6
								},
								"friction_iterations" : 1,
								"friction_convergence_tol": 1e-2,
								"barrier_stiffness": "adaptive",
								"lagged_damping_weight": 0
							}

							"ignore_inertia" : false,

							"advanced": {
								"cache_size" : 900000,
								"lump_mass_matrix" : false
							}
						},

						"PDE": {
							"type" : "Laplacian",
							"default_problem" : "Franke",

							"materials" : null,
						}

						"output": {
							"json" : "",

							"paraview" : {
								"file_name" : "",
								"vismesh_rel_area" : 0.00001,

								"skip_frame" : 1,

								"high_order_mesh" : true,

								"volume" : true,
								"surface" : false,
								"wireframe" : false,

								"options" : {
									"material" : false,
									"body_ids" : false,
									"contact_forces" : false,
									"friction_forces" : false,
									"velocity" : false,
									"acceleration" : false
								}
							},

							"data" : {
								"solution" : "",
								"full_mat" : "",
								"stiffness_mat" : "",
								"solution_mat" : "",
								"stress_mat" : "",
								"u_path" : "",
								"v_path" : "",
								"a_path" : "",
								"mises" : ""
								"nodes" : ""
							},

							"advanced": {
								"timestep_prefix" : "step_"
								"sol_on_grid" : -1,

								"compute_error" : true,

								"sol_at_node" : -1,

								"vis_boundary_only" : false,

								"curved_mesh_size" : false,
								"save_solve_sequence_debug" : false,
								"save_time_sequence" : true,
								"save_nl_solve_sequence" : false,

								"spectrum" : false
							}
						}
					})"_json;
	}

	void State::init_logger(const std::string &log_file, int log_level, const bool is_quiet)
	{
		std::vector<spdlog::sink_ptr> sinks;
		if (!is_quiet)
		{
			sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		}
		if (!log_file.empty())
		{
			sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, /*truncate=*/true));
		}
		init_logger(sinks, log_level);
		spdlog::flush_every(std::chrono::seconds(3));
	}

	void State::init_logger(std::ostream &os, int log_level)
	{
		std::vector<spdlog::sink_ptr> sinks;
		sinks.emplace_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(os, false));
		init_logger(sinks, log_level);
	}

	void State::init_logger(std::vector<spdlog::sink_ptr> &sinks, int log_level)
	{
		spdlog::level::level_enum level =
			static_cast<spdlog::level::level_enum>(std::max(0, std::min(6, log_level)));
		spdlog::set_level(level);

		GEO::Logger *geo_logger = GEO::Logger::instance();
		geo_logger->unregister_all_clients();
		geo_logger->register_client(new GeoLoggerForward(logger().clone("geogram")));
		geo_logger->set_pretty(false);

		IPC_LOG(set_level(level));
	}

	void State::init(const json &p_args_in, const std::string &output_dir)
	{
		json args_in = p_args_in; // mutable copy

		if (args_in.contains("default_params"))
			apply_default_params(args_in);

		check_for_unknown_args(args, args_in);

		this->args.merge_patch(args_in);
		has_dhat = args_in.contains("dhat");

		use_avg_pressure = !args["has_neumann"];

		if (args_in.contains("BDF_order"))
		{
			logger().warn("use time_integrator_params: { num_steps: <value> } instead of BDF_order");
			this->args["time_integrator_params"]["num_steps"] = args_in["BDF_order"];
		}

		if (args_in.contains("stiffness_mat_save_path") && !args_in["stiffness_mat_save_path"].empty())
		{
			logger().warn("use export: { stiffness_mat: 'path' } instead of stiffness_mat_save_path");
			this->args["export"]["stiffness_mat"] = args_in["stiffness_mat_save_path"];
		}

		if (args_in.contains("solution") && !args_in["solution"].empty())
		{
			logger().warn("use export: { solution: 'path' } instead of solution");
			this->args["export"]["solution"] = args_in["solution"];
		}

		if (this->args["has_collision"])
		{
			if (!args_in.contains("line_search"))
			{
				args["line_search"] = "backtracking";
				logger().warn("Changing default linesearch to backtracking");
			}

			if (args["friction_iterations"] == 0)
			{
				logger().info("specified friction_iterations is 0; disabling friction");
				args["mu"] = 0.0;
			}
			else if (args["friction_iterations"] < 0)
			{
				args["friction_iterations"] = std::numeric_limits<int>::max();
			}

			if (args["mu"] == 0.0)
			{
				args["friction_iterations"] = 0;
			}
		}
		else
		{
			args["friction_iterations"] = 0;
			args["mu"] = 0;
		}

		problem = ProblemFactory::factory().get_problem(args["problem"]);
		problem->clear();
		if (args["problem"] == "Kernel")
		{
			KernelProblem &kprob = *dynamic_cast<KernelProblem *>(problem.get());
			kprob.state = this;
		}
		// important for the BC
		problem->set_parameters(args["problem_params"]);

		if (args["use_spline"] && args["n_refs"] == 0)
		{
			logger().warn("n_refs > 0 with spline");
		}

		// Save output directory and resolve output paths dynamically
		this->output_dir = output_dir;
	}

} // namespace polyfem
