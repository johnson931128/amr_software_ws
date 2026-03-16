#ifndef STATE_MACHINE_NODE_HPP_
#define STATE_MACHINE_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include <queue>   // 引入 C++ 佇列函式庫

// 定義 2D 座標點結構
struct Point2D {
    double x;
    double y;
};

enum class AmrState {
    IDLE,
    NAV_TO_WAYPOINT,  // 新增：導航至一般巡邏點
    NAV_TO_ELEVATOR,
    WAIT_ELEVATOR,
    ENTER_ELEVATOR,
    CHANGE_FLOOR_MAP
};

class StateMachineNode : public rclcpp::Node {
public:
    StateMachineNode();

private:
    AmrState current_state_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    // 任務排程器：使用 queue 來儲存待巡邏的座標點
    std::queue<Point2D> waypoint_queue_;

    void update_state();
    std::string state_to_string(AmrState state);
};

#endif  // STATE_MACHINE_NODE_HPP_
