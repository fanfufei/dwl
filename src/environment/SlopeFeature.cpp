#include <environment/SlopeFeature.h>
#include <Eigen/Dense>


namespace dwl
{

namespace environment
{


SlopeFeature::SlopeFeature() : flat_threshold_(0.0 * (M_PI / 180.0)), steep_threshold_(70.0 * (M_PI / 180.0))
{
	name_ = "Slope";
}


SlopeFeature::~SlopeFeature()
{

}


void SlopeFeature::computeReward(double& reward_value, Terrain terrain_info)
{
	double slope = fabs(acos((double) terrain_info.surface_normal(2)));

	if (slope < flat_threshold_)
		reward_value = 0.0;
	else {
		if (slope < steep_threshold_) {
			reward_value = log(fabs((steep_threshold_ - slope) / (steep_threshold_ - flat_threshold_)));
			if (max_reward_ > reward_value)
				reward_value = max_reward_;
		} else
			reward_value = max_reward_;

	}
}

} //@namespace environment
} //@namespace dwl
