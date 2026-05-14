#include "amr_hardware/cmd_vel_to_wheel.hpp"

namespace amr_hardware {

CmdVelToWheelNode::CmdVelToWheelNode(const rclcpp::NodeOptions & options)
: Node("cmd_vel_to_wheel_node", options) {
    
    // 1. 初始化機器參數 (這些數值未來可以寫在 YAML 檔中)
    wheel_base_ = 0.25;   // 輪距 L (公尺)
    wheel_radius_ = 0.033; // 輪半徑 R (公尺)

    // 2. 建立訂閱者：接收來自 Nav2 或手控節點的 /cmd_vel
    cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 10, 
        std::bind(&CmdVelToWheelNode::cmd_vel_callback, this, std::placeholders::_1));

    // 3. 建立發布者：將計算結果送往 micro-ROS (ESP32)
    target_vel_pub_ = this->create_publisher<std_msgs::msg::Float32MultiArray>(
        "esp32/target_vel", 10);

    RCLCPP_INFO(this->get_logger(), "運動學橋接節點 (cmd_vel_to_wheel) 已啟動");
}

void CmdVelToWheelNode::cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    // 取得機器人整體的目標線速度 v 與角速度 w
    double v = msg->linear.x;  // 前進速度 (m/s)
    double w = msg->angular.z; // 自轉速度 (rad/s)

    /**
     * 差速驅動逆運動學公式：
     * 右輪速度 (v_r) = v + (w * L / 2)
     * 左輪速度 (v_l) = v - (w * L / 2)
     * * 轉換成角速度 (omega = v / R) 以便給 ESP32 使用：
     */
    double v_r = v + (w * wheel_base_ / 2.0);
    double v_l = v - (w * wheel_base_ / 2.0);

    double omega_r = v_r / wheel_radius_;
    double omega_l = v_l / wheel_radius_;

    // 打包成 Float32MultiArray 格式
    auto target_vel_msg = std_msgs::msg::Float32MultiArray();
    target_vel_msg.data = {static_cast<float>(omega_l), static_cast<float>(omega_r)};
    
    // 發布訊息
    target_vel_pub_->publish(target_vel_msg);
}

} // namespace amr_hardware

// 註冊為 ROS 2 組件
#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(amr_hardware::CmdVelToWheelNode)
