#include "amr_mission_control/state_machine_node.hpp"

StateMachineNode::StateMachineNode() : Node("state_machine_node"), current_state_(AmrState::IDLE) {
    RCLCPP_INFO(this->get_logger(), "大腦決策層已啟動，準備載入巡邏路線...");

    // 初始化任務排程器：塞入三個巡邏點 (可以想像成實驗室的各個桌子)
    waypoint_queue_.push({1.5, 2.0});
    waypoint_queue_.push({3.0, -1.0});
    waypoint_queue_.push({5.5, 0.0}); // 假設最後一個點是電梯口

    RCLCPP_INFO(this->get_logger(), "成功載入 %zu 個巡邏點。當前狀態: IDLE", waypoint_queue_.size());

    timer_ = this->create_wall_timer(
        std::chrono::seconds(2),
        std::bind(&StateMachineNode::update_state, this));
}

std::string StateMachineNode::state_to_string(AmrState state) {
    switch (state) {
        case AmrState::IDLE: return "IDLE (待機中)";
        case AmrState::NAV_TO_WAYPOINT: return "NAV_TO_WAYPOINT (前往巡邏點)";
        case AmrState::NAV_TO_ELEVATOR: return "NAV_TO_ELEVATOR (前往電梯)";
        case AmrState::WAIT_ELEVATOR: return "WAIT_ELEVATOR (等待開門)";
        case AmrState::ENTER_ELEVATOR: return "ENTER_ELEVATOR (進入電梯)";
        case AmrState::CHANGE_FLOOR_MAP: return "CHANGE_FLOOR_MAP (切換地圖)";
        default: return "UNKNOWN";
    }
}

void StateMachineNode::update_state() {
    switch (current_state_) {
        case AmrState::IDLE:
            if (!waypoint_queue_.empty()) {
                // 取出佇列最前面的座標點
                Point2D target = waypoint_queue_.front();
                waypoint_queue_.pop(); // 移除已取出的點
                
                RCLCPP_INFO(this->get_logger(), "派發新任務！前往座標 (X: %.1f, Y: %.1f)，剩餘 %zu 個任務", target.x, target.y, waypoint_queue_.size());
                
                // 如果是最後一個點，假設它是電梯口
                if (waypoint_queue_.empty()) {
                    current_state_ = AmrState::NAV_TO_ELEVATOR;
                } else {
                    current_state_ = AmrState::NAV_TO_WAYPOINT;
                }
            } else {
                RCLCPP_INFO(this->get_logger(), "所有巡邏任務已完成，待機中...");
            }
            break;
            
        case AmrState::NAV_TO_WAYPOINT:
            RCLCPP_INFO(this->get_logger(), "抵達巡邏點，執行檢測任務後繼續...");
            current_state_ = AmrState::IDLE; // 回到 IDLE 準備接取下一個點
            break;

        case AmrState::NAV_TO_ELEVATOR:
            RCLCPP_INFO(this->get_logger(), "抵達電梯口，等待電梯...");
            current_state_ = AmrState::WAIT_ELEVATOR;
            break;

        case AmrState::WAIT_ELEVATOR:
            RCLCPP_INFO(this->get_logger(), "電梯門開，準備進入...");
            current_state_ = AmrState::ENTER_ELEVATOR;
            break;

        case AmrState::ENTER_ELEVATOR:
            RCLCPP_INFO(this->get_logger(), "已進入電梯，觸發地圖切換程序...");
            current_state_ = AmrState::CHANGE_FLOOR_MAP;
            break;

        case AmrState::CHANGE_FLOOR_MAP:
            RCLCPP_INFO(this->get_logger(), "地圖切換完成，重新進入待機模式。");
            current_state_ = AmrState::IDLE;
            break;
    }
}

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StateMachineNode>());
    rclcpp::shutdown();
    return 0;
}
