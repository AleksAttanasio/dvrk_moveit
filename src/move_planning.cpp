#include <pluginlib/class_loader.h>
#include <ros/ros.h>

// MoveIt
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <moveit/planning_interface/planning_interface.h>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/kinematic_constraints/utils.h>
#include <moveit_msgs/DisplayTrajectory.h>
#include <moveit_msgs/PlanningScene.h>
#include <moveit_visual_tools/moveit_visual_tools.h>

#include <boost/scoped_ptr.hpp>

int main(int argc, char** argv) {

    const std::string node_name = "motion_planning_tutorial";
    ros::init(argc, argv, node_name);
    ros::AsyncSpinner spinner(1);
    spinner.start();
    ros::NodeHandle node_handle("~");

    // Create a RobotState and JointModelGroup to keep track of the current robot pose and planning group
    const std::string PLANNING_GROUP = "psm_arm";
    robot_model_loader::RobotModelLoader robot_model_loader("robot_description");
    robot_model::RobotModelPtr robot_model = robot_model_loader.getModel();
    robot_state::RobotStatePtr robot_state(new robot_state::RobotState(robot_model));
    const robot_state::JointModelGroup* joint_model_group = robot_state->getJointModelGroup(PLANNING_GROUP);

    planning_scene::PlanningScenePtr planning_scene(new planning_scene::PlanningScene(robot_model));
    planning_scene->getCurrentStateNonConst().setToDefaultValues(joint_model_group, "ready");

    // Load plugin planner
    boost::scoped_ptr<pluginlib::ClassLoader<planning_interface::PlannerManager>> planner_plugin_loader;
    planning_interface::PlannerManagerPtr planner_instance;
    std::string planner_plugin_name;

    // Handle plugin boot
    if (!node_handle.getParam("/move_group/planning_plugin", planner_plugin_name))
        ROS_FATAL_STREAM("Could not find planner plugin name");
    try
    {
        planner_plugin_loader.reset(new pluginlib::ClassLoader<planning_interface::PlannerManager>(
                "moveit_core", "planning_interface::PlannerManager")); // !!!!!!!!!!!!!!! Controllare field package (forse ci va stomp_ros)
    }
    catch (pluginlib::PluginlibException& ex)
    {
        ROS_FATAL_STREAM("Exception while creating planning plugin loader " << ex.what());
    }
    try
    {
        planner_instance.reset(planner_plugin_loader->createUnmanagedInstance(planner_plugin_name));
        if (!planner_instance->initialize(robot_model, node_handle.getNamespace()))
            ROS_FATAL_STREAM("Could not initialize planner instance");
        ROS_INFO_STREAM("Using planning interface '" << planner_instance->getDescription() << "'");
    }
    catch (pluginlib::PluginlibException& ex)
    {
        const std::vector<std::string>& classes = planner_plugin_loader->getDeclaredClasses();
        std::stringstream ss;
        for (std::size_t i = 0; i < classes.size(); ++i)
            ss << classes[i] << " ";
        ROS_ERROR_STREAM("Exception while loading planner '" << planner_plugin_name << "': " << ex.what() << std::endl
                                                             << "Available plugins: " << ss.str());
    }

    // ################
    // ### VISUALIS ###
    // ################
    namespace rvt = rviz_visual_tools;
    moveit_visual_tools::MoveItVisualTools visual_tools("world");
    visual_tools.loadRobotStatePub("/display_robot_state");
    visual_tools.enableBatchPublishing();
    visual_tools.deleteAllMarkers();  // clear all old markers
    visual_tools.trigger();

    visual_tools.loadRemoteControl();

    // Batch publishing is used to reduce the number of messages being sent to RViz for large visualizations
    visual_tools.trigger();

    // ################
    // ### PLANNING ###
    // ################
    visual_tools.publishRobotState(planning_scene->getCurrentStateNonConst(), rviz_visual_tools::GREEN);
    visual_tools.trigger();
    planning_interface::MotionPlanRequest req;
    planning_interface::MotionPlanResponse res;


    geometry_msgs::PoseStamped pose;
//    pose.header.frame_id = "world";
    pose.pose.position.x = 0.05;
    pose.pose.position.y = 0.05;
    pose.pose.position.z = -0.15;
    pose.pose.orientation.w = 1.0;


    std::vector<double> tolerance_pose(3, 0.01);
    std::vector<double> tolerance_angle(3, 0.01);

    moveit_msgs::Constraints pose_goal =
            kinematic_constraints::constructGoalConstraints("psm_tool_tip_link", pose, tolerance_pose, tolerance_angle);


    req.group_name = PLANNING_GROUP;
    req.goal_constraints.push_back(pose_goal);


    planning_interface::PlanningContextPtr context = planner_instance->getPlanningContext(planning_scene, req, res.error_code_);
    context->solve(res);
    if (res.error_code_.val != res.error_code_.SUCCESS)
    {
        ROS_ERROR("Could not compute plan successfully");
        return 0;
    }

    ros::Publisher display_publisher =
            node_handle.advertise<moveit_msgs::DisplayTrajectory>("/move_group/display_planned_path", 1, true);
    moveit_msgs::DisplayTrajectory display_trajectory;

    // Trajectory visualization
    moveit_msgs::MotionPlanResponse response;
    res.getMessage(response);
    display_trajectory.trajectory_start = response.trajectory_start;
    display_trajectory.trajectory.push_back(response.trajectory);
    visual_tools.publishTrajectoryLine(display_trajectory.trajectory.back(), joint_model_group);
    visual_tools.trigger();
    display_publisher.publish(display_trajectory);

    robot_state->setJointGroupPositions(joint_model_group, response.trajectory.joint_trajectory.points.back().positions);
    planning_scene->setCurrentState(*robot_state.get());

    visual_tools.publishRobotState(planning_scene->getCurrentStateNonConst(), rviz_visual_tools::GREEN);
    visual_tools.publishAxisLabeled(pose.pose, "goal_1");

    visual_tools.trigger();

    visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to continue the demo");

    // Second target and planning
//    geometry_msgs::PoseStamped pose_end;
//    pose_end.header.frame_id = "world";
//    pose_end.pose.position.x = 0.1;
//    pose_end.pose.position.y = 0.1;
//    pose_end.pose.position.z = -0.10;
//    pose_end.pose.orientation.w = 1.0;
//
//    moveit_msgs::Constraints pose_goal_end =
//            kinematic_constraints::constructGoalConstraints("psm_tool_tip_link", pose_end, tolerance_pose, tolerance_angle);
//
//    req.goal_constraints.clear();
//    req.goal_constraints.push_back(pose_goal_end);
//    context = planner_instance->getPlanningContext(planning_scene, req, res.error_code_);
//    context->solve(res);
//
//    if (res.error_code_.val != res.error_code_.SUCCESS)
//    {
//        ROS_ERROR("Could not compute plan successfully");
//        return 0;
//    }
//
//    res.getMessage(response);
//    display_trajectory.trajectory.push_back(response.trajectory);
//    visual_tools.publishTrajectoryLine(display_trajectory.trajectory.back(), joint_model_group);
//    visual_tools.trigger();
//    display_publisher.publish(display_trajectory);
//
//    robot_state->setJointGroupPositions(joint_model_group, response.trajectory.joint_trajectory.points.back().positions);
//    planning_scene->setCurrentState(*robot_state.get());
//
//    // Display the goal state
//    visual_tools.publishRobotState(planning_scene->getCurrentStateNonConst(), rviz_visual_tools::GREEN);
//    visual_tools.publishAxisLabeled(pose_end.pose, "goal_2");
//    visual_tools.trigger();
//
//    /* Wait for user input */
//    visual_tools.prompt("Press 'next' in the RvizVisualToolsGui window to continue the demo");




}

