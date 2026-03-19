#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <chrono>

using namespace std::chrono_literals;

class SquareTester : public rclcpp::Node {
public:
    SquareTester() : Node("square_tester"), state_(0), tick_count_(0) {
        // 1. 建立 Publisher，對接 Gazebo 裡的差速外掛
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
        
        // 2. 建立 Timer，每 0.1 秒 (100ms) 觸發一次 timer_callback
        timer_ = this->create_wall_timer(
            100ms, std::bind(&SquareTester::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "走正方形測試節點已啟動！準備出發...");
    }

private:
    void timer_callback() {
        auto msg = geometry_msgs::msg::Twist();

        // 簡單狀態機：state 0 是直走，state 1 是轉彎
        if (state_ == 0) {
            msg.linear.x = 0.2;  // 往前速度 0.2 m/s
            msg.angular.z = 0.0;
            tick_count_++;
            
            // 0.1秒 * 30次 = 3秒。直走 3 秒後切換為轉彎
            if (tick_count_ >= 30) {
                state_ = 1;
                tick_count_ = 0;
                RCLCPP_INFO(this->get_logger(), "到達頂點，開始 90 度轉彎！");
            }
        } else {
            msg.linear.x = 0.0;
            msg.angular.z = 0.785; // 角速度大約 pi/4 rad/s
            tick_count_++;
            
            // 0.1秒 * 20次 = 2秒。轉彎 2 秒 (0.785 * 2 = 1.57 rad，約 90 度)
            if (tick_count_ >= 20) {
                state_ = 0;
                tick_count_ = 0;
                RCLCPP_INFO(this->get_logger(), "轉彎完成，繼續直走！");
            }
        }
        
        // 發布速度指令給底層
        publisher_->publish(msg);
    }

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int state_;       // 紀錄目前狀態 (0:直走, 1:轉彎)
    int tick_count_;  // 紀錄經過了幾個 0.1 秒
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SquareTester>());
    rclcpp::shutdown();
    return 0;
}
