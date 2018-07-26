#include <dwl/WholeBodyState.h>
#include <dwl/model/WholeBodyKinematics.h>


using namespace std;

int main(int argc, char **argv)
{
	dwl::WholeBodyState ws;
	dwl::model::FloatingBaseSystem fbs;
	dwl::model::WholeBodyKinematics wkin;

	// Resetting the system from the hyq urdf file
	std::string urdf_file = DWL_SOURCE_DIR"/models/hyq.urdf";
	std::string yarf_file = DWL_SOURCE_DIR"/models/hyq.yarf";
	fbs.resetFromURDFFile(urdf_file, yarf_file);
	wkin.reset(fbs);

	// Define the DoF after initializing the robot model
	ws.setJointDoF(fbs.getJointDoF());


	// The robot state
	ws.setBaseSE3(dwl::SE3(Eigen::Vector3d(0., 0., 0.),
						   Eigen::Vector3d(0., 0., 0.)));
	ws.setBaseVelocity_W(dwl::Motion(Eigen::Vector3d(0., 0., 0.),
									 Eigen::Vector3d(0., 0., 0.)));
	ws.setBaseAcceleration_W(dwl::Motion(Eigen::Vector3d(0., 0., 0.),
										 Eigen::Vector3d(0., 0., 0.)));
	ws.setJointPosition(0.75, fbs.getJointId("lf_hfe_joint"));
	ws.setJointPosition(-1.5, fbs.getJointId("lf_kfe_joint"));
	ws.setJointPosition(-0.75, fbs.getJointId("lh_hfe_joint"));
	ws.setJointPosition(1.5, fbs.getJointId("lh_kfe_joint"));
	ws.setJointPosition(0.75, fbs.getJointId("rf_hfe_joint"));
	ws.setJointPosition(-1.5, fbs.getJointId("rf_kfe_joint"));
	ws.setJointPosition(-0.75, fbs.getJointId("rh_hfe_joint"));
	ws.setJointPosition(1.5, fbs.getJointId("rh_kfe_joint"));

	ws.setJointAcceleration(1., fbs.getJointId("lf_hfe_joint"));

	
	// Computing the CoM position and velocity
	Eigen::Vector3d x, xd;
	wkin.computeCoMRate(x, xd,
						ws.base_pos, ws.joint_pos,
						ws.base_vel, ws.joint_vel);
	cout << "x = " << x.transpose() << endl;
	cout << "xd = " << xd.transpose() << endl << endl;

	// Computing the Cartesian position of a set of frames
	dwl::SE3Map pos_W =
		wkin.computePosition(ws.base_pos, ws.joint_pos,
							 fbs.getEndEffectorNames());
	cout << "Frame position:" << endl;
	for (dwl::SE3Map::iterator it = pos_W.begin();
		it != pos_W.end(); ++it) {
			cout << "  " << it->first;
			cout << "  t = " << it->second.getTranslation().transpose() << endl;
			cout << "  R = " << it->second.getRotation().row(0) << endl;
			cout << "      " << it->second.getRotation().row(1) << endl;
			cout << "      " << it->second.getRotation().row(2);
	}
	cout << endl;
	// ws.setContactPosition_W(pos_W);


	// Computing the Cartesian velocity of a set of frames. A frame is a fixed-point
	// in a body
	dwl::MotionMap vel_W =
		wkin.computeVelocity(ws.base_pos, ws.joint_pos,
							 ws.base_vel, ws.joint_vel,
							 fbs.getEndEffectorNames());
	cout << "Frame velocity:" << endl;
	for (dwl::MotionMap::iterator it = vel_W.begin();
		it != vel_W.end(); ++it) {
			cout << "  " << it->first;
			cout << "  v = " << it->second.getLinear().transpose() << endl;
			cout << "  w = " << it->second.getAngular().transpose() << endl;
	}
	cout << endl;
	// ws.setContactVelocity_W(pos_W);
	

	// Computing the Cartesian acceleration of a set of frames
	dwl::MotionMap acc_W =
		wkin.computeAcceleration(ws.base_pos, ws.joint_pos,
								 ws.base_vel, ws.joint_vel,
								 ws.base_acc, ws.joint_acc,
								 fbs.getEndEffectorNames());
	cout << "Frame acceleration:" << endl;
	for (dwl::MotionMap::iterator it = acc_W.begin();
		it != acc_W.end(); ++it) {
			cout << "  " << it->first;
			cout << "  v = " << it->second.getLinear().transpose() << endl;
			cout << "  w = " << it->second.getAngular().transpose() << endl;
	}
	cout << endl;
	// ws.setContactAcceleration_W(pos_W);
	
	
	// Computing the contact Jacd*Qd
	dwl::MotionMap jd_qd_W =
		wkin.computeJdQd(ws.base_pos, ws.joint_pos,
						 ws.base_vel, ws.joint_vel,
						 fbs.getEndEffectorNames());
	cout << "Frame Jd*qd term:" << endl;
	for (dwl::MotionMap::iterator it = jd_qd_W.begin();
		it != jd_qd_W.end(); ++it) {
			cout << "  " << it->first;
			cout << "  v = " << it->second.getLinear().transpose() << endl;
			cout << "  w = " << it->second.getAngular().transpose() << endl;
	}
	cout << endl;

	// Computing the Jacobians
	Eigen::MatrixXd jacobian, fixed_jac, floating_jac;
	Eigen::Matrix6xMap jac = 
		wkin.computeJacobian(ws.base_pos, ws.joint_pos,
							 fbs.getEndEffectorNames());
	cout << "Frame Jacobian:" << endl;
	for (Eigen::Matrix6xMap::iterator it = jac.begin();
		it != jac.end(); ++it) {
			cout << it->first << ":" << endl;
			cout << it->second << endl;
			cout << "---" << endl;
			Eigen::Matrix6d floating_jac;
			Eigen::Matrix6x fixed_jac;
			wkin.getFloatingBaseJacobian(floating_jac, it->second);
			wkin.getFixedBaseJacobian(fixed_jac, it->second);
			cout << floating_jac << " = Floating-base Jacobian" << endl;
			cout << "---" << endl;
			cout << fixed_jac << " = Fixed-base Jacobian" << endl;
			cout << endl;
	}
	cout << endl;
	cout << "---------------------------------------" << endl;



	// Computing IK
	dwl::SE3Map frame_pos;// = pos_W;
	frame_pos["lf_foot"] = pos_W.find("lf_foot")->second;
	frame_pos["lh_foot"] = pos_W.find("lh_foot")->second;
	wkin.setIKSolver( 1.0e-12, 50);
	Eigen::VectorXd joint_pos0(12);
	joint_pos0 << 0., 0.5, -1., 0., -0.5, 1., 0., 0.5, -1., 0., -0.5, 1.;
	if (wkin.computeJointPosition(ws.joint_pos,
								  frame_pos,
								  joint_pos0)) {
		cout << "Joint position = "<< ws.joint_pos.transpose() << endl << endl;
	} else {
		cout << "The IK problem could not be solved" << endl;
	}

	dwl::MotionMap frame_vel;
	frame_vel["lf_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(1., 0., 0.));
	frame_vel["lh_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(0., 1., 0.));
	frame_vel["rf_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(0., 0., 1.));
	wkin.computeJointVelocity(ws.joint_vel, ws.joint_pos, frame_vel);
	cout << "Joint velocity = " << ws.joint_vel.transpose() << endl << endl;


	dwl::MotionMap frame_acc;
	frame_acc["lf_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(1., 0., 0.));
	frame_acc["lh_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(0., 1., 0.));
	frame_acc["rf_foot"] = dwl::Motion(Eigen::Vector3d::Zero(),
									   Eigen::Vector3d(0., 0., 1.));
	wkin.computeJointAcceleration(ws.joint_acc, ws.joint_pos, ws.joint_vel, frame_acc);
	cout << "Joint acceleration = " << ws.joint_acc.transpose() << endl << endl;


    return 0;
}
