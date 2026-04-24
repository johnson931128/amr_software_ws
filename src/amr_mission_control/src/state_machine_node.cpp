#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "nav2_msgs/srv/load_map.hpp"
#include <chrono>
#include <string>

using namespace std::chrono_literals;

enum class AmrState {
    WAIT_ELEVATOR,
    NAV_TO_ELEVATOR,
    CHANGE_FLOOR_MAP,
    IDLE
};

class StateMachineNode : public rclcpp::Node {
public:
    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandleNavigateToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;
    using LoadMap = nav2_msgs::srv::LoadMap;

    StateMachineNode() : Node("state_machine_node"), current_state_(AmrState::WAIT_ELEVATOR) {
        RCLCPP_INFO(this->get_logger(), "AMR State Machine Brain Initialized.");

        // Initialize Action Client for Nav2
        nav_client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");

        // Initialize Service Client for Map Server
        map_client_ = this->create_client<LoadMap>("/map_server/load_map");

        // Main Control Loop Timer
        timer_ = this->create_wall_timer(2s, [this]() {
            this->update_state();
        });
    }

private:
    AmrState current_state_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp_action::Client<NavigateToPose>::SharedPtr nav_client_;
    rclcpp::Client<LoadMap>::SharedPtr map_client_;

    std::string state_to_string(AmrState state) {
        switch (state) {
            case AmrState::WAIT_ELEVATOR:    return "WAIT_ELEVATOR";
            case AmrState::NAV_TO_ELEVATOR:  return "NAV_TO_ELEVATOR";
            case AmrState::CHANGE_FLOOR_MAP: return "CHANGE_FLOOR_MAP";
            case AmrState::IDLE:             return "IDLE";
            default:                         return "UNKNOWN";
        }
    }

    void update_state() {
        RCLCPP_INFO(this->get_logger(), "Current State: %s", state_to_string(current_state_).c_str());

        switch (current_state_) {
            case AmrState::WAIT_ELEVATOR:
                send_nav_goal();
                current_state_ = AmrState::NAV_TO_ELEVATOR;
                break;

            case AmrState::NAV_TO_ELEVATOR:
                // Waiting for Action result callback
                break;

            case AmrState::CHANGE_FLOOR_MAP:
                switch_map("/home/fanshunjie/amr_ws/maps/B1_map.yaml");
                current_state_ = AmrState::IDLE;
                break;

            case AmrState::IDLE:
                break;
        }
    }

    // --- Action Client Logic ---
    void send_nav_goal() {
        if (!nav_client_->wait_for_action_server(5s)) {
            RCLCPP_ERROR(this->get_logger(), "Nav2 Action server not available.");
            return;
        }

        auto goal_msg = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();
        goal_msg.pose.pose.position.x = 2.7;
        goal_msg.pose.pose.position.y = -7.6;
        goal_msg.pose.pose.orientation.w = 1.0;

        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        send_goal_options.result_callback = [this](const GoalHandleNavigateToPose::WrappedResult & result) {
            if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
                RCLCPP_INFO(this->get_logger(), "Arrived at elevator!");
                this->current_state_ = AmrState::CHANGE_FLOOR_MAP;
            } else {
                RCLCPP_ERROR(this->get_logger(), "Navigation failed.");
                this->current_state_ = AmrState::IDLE;
            }
        };

        RCLCPP_INFO(this->get_logger(), "Sending navigation goal...");
        nav_client_->async_send_goal(goal_msg, send_goal_options);
    }

    // --- Service Client Logic ---
    void switch_map(const std::string & map_path) {
        if (!map_client_->wait_for_service(5s)) {
            RCLCPP_ERROR(this->get_logger(), "Map load service not available.");
            return;
        }

        auto request = std::make_shared<LoadMap::Request>();
        request->map_url = map_path;

        map_client_->async_send_request(request, [this](rclcpp::Client<LoadMap>::SharedFuture future) {
            auto response = future.get();
            if (response->result == LoadMap::Response::RESULT_SUCCESS) {
                RCLCPP_INFO(this->get_logger(), "B1 Map Loaded Successfully!");
            } else {
                RCLCPP_ERROR(this->get_logger(), "Failed to load B1 Map.");
            }
        });
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StateMachineNode>());
    rclcpp::shutdown();
    return 0;
}
