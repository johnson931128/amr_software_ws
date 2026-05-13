#ifndef AMR_HARDWARE__CMD_VEL_TO_WHEEL_HPP_
#define AMR_HARDWARE__CMD_VEL_TO_WHEEL_HPP_

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"

namespace amr_hardware {

class CmdVelToWheelNode : public rclcpp::Node {
public:
    // 建構子，支援 ROS 2 Component 動態載入與參數注入
    explicit CmdVelToWheelNode(const rclcpp::NodeOptions & options);

private:
    // 回呼函式：當收到 Nav2 的 cmd_vel 時觸發
    void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg);

    // 訂閱者 (Subscriber)：監聽 Nav2 發出的整體目標速度 (Twist)
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    
    // 發布者 (Publisher)：發布左右輪目標轉速給 ESP32 (Float32MultiArray)
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr target_vel_pub_;

    // 機器人物理參數 (用於逆運動學計算)
    double wheel_base_;   // 輪距 L (公尺) - 左右輪中心的距離
    double wheel_radius_; // 輪半徑 R (公尺)
};

} // namespace amr_hardware

#endif // AMR_HARDWARE__CMD_VEL_TO_WHEEL_HPP_
