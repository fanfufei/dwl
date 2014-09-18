#ifndef DWL_Planners_HierarchicalPlanner_H
#define DWL_Planners_HierarchicalPlanner_H

#include <ros/ros.h>

// Planning headers
#include <planning/WholeBodyLocomotion.h>
#include <planning/HierarchicalPlanning.h>
#include <planning/MotionPlanning.h>
#include <planning/SearchBasedBodyMotionPlanning.h>
#include <planning/ContactPlanning.h>
#include <planning/GreedyFootstepPlanning.h>
#include <planning/Dijkstrap.h>
#include <planning/AStar.h>
#include <planning/AnytimeRepairingAStar.h>

// Robot and Environment headers
#include <robot/Robot.h>
#include <environment/EnvironmentInformation.h>
#include <environment/AdjacencyEnvironment.h>
#include <environment/GridBasedBodyAdjacency.h>
#include <environment/LatticeBasedBodyAdjacency.h>
#include <environment/PotentialLegCollisionFeature.h>
#include <environment/PotentialBodyOrientationFeature.h>
#include <environment/SupportTriangleFeature.h>
#include <environment/LegCollisionFeature.h>
#include <environment/BodyOrientationFeature.h>

// Messages headers
#include <dwl_planners/ContactSequence.h>
#include <terrain_server/RewardMap.h>
#include <terrain_server/ObstacleMap.h>
#include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/Path.h>
#include <visualization_msgs/Marker.h>

// TF headers
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/message_filter.h>
#include <message_filters/subscriber.h>


namespace dwl_planners
{

/**
 * @class HierarchicalPlanners
 * @brief Class for solving legged motion planning by approaching as a hierarchical planner
 */
class HierarchicalPlanners
{
	public:
		/**
		 * @brief Constructor function
		 * @param ros::NodeHandle node ROS node handle
		 */
		HierarchicalPlanners(ros::NodeHandle node);

		/** @brief Destructor function */
		~HierarchicalPlanners();

		/** @brief Initializes the hierarchical planner */
		void init();

		void initBodyPlanner();
		void initContactPlanner();

		/** @brief Computes the hierarchical plan */
		bool compute();

		/**
		 * @brief Callback method when the reward map message arrives
		 * @param const terrain_server::RewardMapConstPtr& msg Reward map message
		 */
		void rewardMapCallback(const terrain_server::RewardMapConstPtr& msg);

		/**
		 * @brief Callback method when the obstacle map message arrives
		 * @param const terrain_server::ObstacleMapConstPtr& msg Obstacle map message
		 */
		void obstacleMapCallback(const terrain_server::ObstacleMapConstPtr& msg);

		/**
		 * @brief Callback method for reseting the body goal state
		 * @param const geometry_msgs::PoseStampedConstPtr& msg Body goal message
		 */
		void resetGoalCallback(const geometry_msgs::PoseStampedConstPtr& msg);

		/** @brief Publishes the computed body path */
		void publishBodyPath();

		/** @brief Publishes the contact sequence */
		void publishContactSequence();


	private:
		/** @brief ROS node handle */
		ros::NodeHandle node_;

		/** @brief Reward map subscriber */
		//message_filters::Subscriber<terrain_server::RewardMap>* reward_sub_;

		/** @brief TF listener */
		tf::TransformListener tf_listener_;

		/** @brief Reward map subscriber */
		ros::Subscriber reward_sub_;

		/** @brief Obstacle map subscriber */
		ros::Subscriber obstacle_sub_;

		/** @brief Body goal subscriber */
		ros::Subscriber body_goal_sub_;

		/** @brief Approximated body path publisher */
		ros::Publisher body_path_pub_;

		/** @brief Contact sequence publisher */
		ros::Publisher contact_sequence_pub_;

		/** @brief Contact sequence publisher for visualization */
		ros::Publisher contact_sequence_rviz_pub_;

		/** @brief Thread mutex of the reward information */
		pthread_mutex_t reward_lock_;

		/** @brief Thread mutex of the obstacle information */
		pthread_mutex_t obstacle_lock_;

		/** @brief Thread mutex of the planner */
		pthread_mutex_t planner_lock_;

		/** @brief Locomotion algorithm */
		dwl::WholeBodyLocomotion locomotor_;

		/** @brief Planning of motion sequences pointer */
		dwl::planning::PlanningOfMotionSequences* planning_ptr_;

		/** @brief Body planner */
		dwl::planning::MotionPlanning* body_planner_ptr_;

		/** @brief Contact planner */
		dwl::planning::ContactPlanning* footstep_planner_ptr_;

		dwl::environment::AdjacencyEnvironment* adjacency_ptr_;

		/** @brief Environment information */
		dwl::environment::EnvironmentInformation environment_;

		/** @brief Robot properties */
		dwl::robot::Robot robot_;

		/** @brief Solver pointer */
		dwl::planning::Solver* body_path_solver_ptr_;

		/** @brief Current robot pose */
		dwl::Pose current_pose_;

		/** @brief Approximated body path message */
		nav_msgs::Path body_path_msg_;

		/** @brief Contact sequence ROS message */
		dwl_planners::ContactSequence contact_sequence_msg_;

		/** @brief Contact sequence for visualization in RVIZ */
		visualization_msgs::Marker contact_sequence_rviz_msg_;

		/** @brief Approximated body path */
		std::vector<dwl::Pose> body_path_;

		/** Contact sequence */
		std::vector<dwl::Contact> contact_sequence_;

		/** @brief Base frame */
		std::string base_frame_;

		/** @brief World frame */
		std::string world_frame_;
};

} //@namespace dwl_planners

#endif
