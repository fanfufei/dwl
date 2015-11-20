#include <dwl/model/Constraint.h>


namespace dwl
{

namespace model
{

Constraint::Constraint() : constraint_dimension_(0), is_soft_(false)
{
	state_buffer_.set_capacity(4);
}


Constraint::~Constraint()
{

}


void Constraint::modelFromURDFFile(std::string filename,
								   bool info)
{
	// Reading the file
	std::ifstream model_file(filename.c_str());
	if (!model_file) {
		std::cerr << "Error opening file '" << filename << "'." << std::endl;
		abort();
	}

	// Reserving memory for the contents of the file
	std::string model_xml_string;
	model_file.seekg(0, std::ios::end);
	model_xml_string.reserve(model_file.tellg());
	model_file.seekg(0, std::ios::beg);
	model_xml_string.assign((std::istreambuf_iterator<char>(model_file)),
			std::istreambuf_iterator<char>());
	model_file.close();

	modelFromURDFModel(model_xml_string, info);
}


void Constraint::modelFromURDFModel(std::string urdf_model,
									bool info)
{
	// Reseting the floating-base system information given an URDF model
	system_.resetFromURDFModel(urdf_model);

	// Initializing the kinematical and dynamical model from the URDF model
	kinematics_.modelFromURDFModel(urdf_model, info);
	dynamics_.modelFromURDFModel(urdf_model, false);

	// Initializing the information of the specific constraint
	init(urdf_model);
}


void Constraint::init(std::string urdf_model,
					  bool info)
{

}


void Constraint::computeSoft(double& constraint_cost,
							 const WholeBodyState& state)
{
	// Getting the constraint value and bounds
	Eigen::VectorXd constraint, lower_bound, upper_bound;
	compute(constraint, state);

	getBounds(lower_bound, upper_bound);

	// Checking unbound values
	unsigned int bound_dim = lower_bound.size();
	for (unsigned int i = 0; i < bound_dim; i++) {
		if (lower_bound(i) == NO_BOUND)
			lower_bound(i) = constraint(i);
		if (upper_bound(i) == NO_BOUND)
			upper_bound(i) = constraint(i);
	}

	// Computing a quadratic cost of the constraint violation
	double weight = 10000
	Eigen::VectorXd lower_violation = lower_bound - constraint;
	Eigen::VectorXd upper_violation = constraint - upper_bound;
	constraint_cost = weight * (lower_violation.norm() + upper_violation.norm());
}


bool Constraint::isSoftConstraint()
{
	return is_soft_;
}


void Constraint::setLastState(WholeBodyState& last_state)
{
	state_buffer_.push_front(last_state);
}


void Constraint::resetStateBuffer()
{
	unsigned int buffer_size = state_buffer_.size();
	WholeBodyState empty_state(state_buffer_[0].joint_pos.size());
	for (unsigned int i = 0; i < buffer_size; i++)
		state_buffer_.push_back();
}


unsigned int Constraint::getConstraintDimension()
{
	// Getting the constraint dimension given a defined constraint function.
	Eigen::VectorXd bound;
	getBounds(bound, bound);
	constraint_dimension_ = bound.size();

	return constraint_dimension_;
}


std::string Constraint::getName()
{
	return name_;
}

} //@namespace model
} //@namespace dwl
