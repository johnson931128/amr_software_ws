#include "amr_hardware/amr_system.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

// 用於字串處理與 Linux Serial 通訊 (此處為架構示意，實作可搭配 termios 或第三方 Serial 庫)
#include <sstream>
#include <iostream>

namespace amr_hardware
{

hardware_interface::CallbackReturn AmrSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  // 1. 初始化基礎框架
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // 2. 初始化儲存輪速的陣列 (假設只有左右兩輪)
  hw_states_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_commands_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());

  // 3. 從 URDF 解析 Serial Port 參數 (例如 /dev/ttyUSB0)
  // std::string serial_port = info_.hardware_parameters["serial_port"];
  
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "硬體初始化成功，準備連線 ESP32...");
  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> AmrSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  // 向 ROS 2 註冊：我會提供左右輪的「實際速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_states_[i]));
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> AmrSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  // 向 ROS 2 註冊：我開放讓 Controller 寫入左右輪的「目標速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_commands_[i]));
  }
  return command_interfaces;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "啟動硬體連線！(此處實作 Linux open() 開啟 Serial Port)");
  
  // TODO: 透過 Linux <termios.h> 開啟 /dev/ttyUSB0 並設定鮑率為 115200
  // 為了防止一開機暴衝，先將指令歸零
  for (auto & cmd : hw_commands_) {
    cmd = 0.0;
  }
  for (auto & state : hw_states_) {
    state = 0.0;
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "關閉硬體連線！(此處實作 Linux close() 關閉 Serial Port)");
  // TODO: 發送停止指令給 ESP32，並關閉 File Descriptor
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type AmrSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Controller 會以極快頻率 (例如 50Hz) 呼叫此函式
  // TODO: 透過 read() 從 Serial Port 讀取 ESP32 傳來的 JSON 或字串
  // 假設收到的字串為: "V,1.5,1.5\n" (代表左右輪速皆為 1.5 rad/s)
  
  // 這裡放入假資料作為測試，實際上應該解析 Serial 數據並更新 hw_states_
  // hw_states_[0] = left_wheel_velocity_from_esp32;
  // hw_states_[1] = right_wheel_velocity_from_esp32;

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type AmrSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // 將 ROS 2 導航演算法算出的目標輪速 (hw_commands_)，轉成字串發給 ESP32
  // 例如 hw_commands_[0] 是左輪目標，hw_commands_[1] 是右輪目標
  
  std::stringstream ss;
  ss << "CMD," << hw_commands_[0] << "," << hw_commands_[1] << "\n";
  std::string command_str = ss.str();

  // TODO: 透過 write() 將 command_str 發送進 Serial Port
  // RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "發送指令: %s", command_str.c_str());

  return hardware_interface::return_type::OK;
}

}  // namespace amr_hardware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  amr_hardware::AmrSystemHardware, hardware_interface::SystemInterface)i#include "amr_hardware/amr_system.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

// 用於字串處理與 Linux Serial 通訊 (此處為架構示意，實作可搭配 termios 或第三方 Serial 庫)
#include <sstream>
#include <iostream>

namespace amr_hardware
{

hardware_interface::CallbackReturn AmrSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  // 1. 初始化基礎框架
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // 2. 初始化儲存輪速的陣列 (假設只有左右兩輪)
  hw_states_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_commands_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());

  // 3. 從 URDF 解析 Serial Port 參數 (例如 /dev/ttyUSB0)
  // std::string serial_port = info_.hardware_parameters["serial_port"];
  
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "硬體初始化成功，準備連線 ESP32...");
  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> AmrSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  // 向 ROS 2 註冊：我會提供左右輪的「實際速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_states_[i]));
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> AmrSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  // 向 ROS 2 註冊：我開放讓 Controller 寫入左右輪的「目標速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_commands_[i]));
  }
  return command_interfaces;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "啟動硬體連線！(此處實作 Linux open() 開啟 Serial Port)");
  
  // TODO: 透過 Linux <termios.h> 開啟 /dev/ttyUSB0 並設定鮑率為 115200
  // 為了防止一開機暴衝，先將指令歸零
  for (auto & cmd : hw_commands_) {
    cmd = 0.0;
  }
  for (auto & state : hw_states_) {
    state = 0.0;
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "關閉硬體連線！(此處實作 Linux close() 關閉 Serial Port)");
  // TODO: 發送停止指令給 ESP32，並關閉 File Descriptor
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type AmrSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Controller 會以極快頻率 (例如 50Hz) 呼叫此函式
  // TODO: 透過 read() 從 Serial Port 讀取 ESP32 傳來的 JSON 或字串
  // 假設收到的字串為: "V,1.5,1.5\n" (代表左右輪速皆為 1.5 rad/s)
  
  // 這裡放入假資料作為測試，實際上應該解析 Serial 數據並更新 hw_states_
  // hw_states_[0] = left_wheel_velocity_from_esp32;
  // hw_states_[1] = right_wheel_velocity_from_esp32;

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type AmrSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // 將 ROS 2 導航演算法算出的目標輪速 (hw_commands_)，轉成字串發給 ESP32
  // 例如 hw_commands_[0] 是左輪目標，hw_commands_[1] 是右輪目標
  
  std::stringstream ss;
  ss << "CMD," << hw_commands_[0] << "," << hw_commands_[1] << "\n";
  std::string command_str = ss.str();

  // TODO: 透過 write() 將 command_str 發送進 Serial Port
  // RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "發送指令: %s", command_str.c_str());

  return hardware_interface::return_type::OK;
}

}  // namespace amr_hardware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  amr_hardware::AmrSystemHardware, hardware_interface::SystemInterface)i#include "amr_hardware/amr_system.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

// 用於字串處理與 Linux Serial 通訊 (此處為架構示意，實作可搭配 termios 或第三方 Serial 庫)
#include <sstream>
#include <iostream>

namespace amr_hardware
{

hardware_interface::CallbackReturn AmrSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  // 1. 初始化基礎框架
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // 2. 初始化儲存輪速的陣列 (假設只有左右兩輪)
  hw_states_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_commands_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());

  // 3. 從 URDF 解析 Serial Port 參數 (例如 /dev/ttyUSB0)
  // std::string serial_port = info_.hardware_parameters["serial_port"];
  
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "硬體初始化成功，準備連線 ESP32...");
  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> AmrSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  // 向 ROS 2 註冊：我會提供左右輪的「實際速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_states_[i]));
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> AmrSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  // 向 ROS 2 註冊：我開放讓 Controller 寫入左右輪的「目標速度 (Velocity)」
  for (uint i = 0; i < info_.joints.size(); i++) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_commands_[i]));
  }
  return command_interfaces;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "啟動硬體連線！(此處實作 Linux open() 開啟 Serial Port)");
  
  // TODO: 透過 Linux <termios.h> 開啟 /dev/ttyUSB0 並設定鮑率為 115200
  // 為了防止一開機暴衝，先將指令歸零
  for (auto & cmd : hw_commands_) {
    cmd = 0.0;
  }
  for (auto & state : hw_states_) {
    state = 0.0;
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn AmrSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "關閉硬體連線！(此處實作 Linux close() 關閉 Serial Port)");
  // TODO: 發送停止指令給 ESP32，並關閉 File Descriptor
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type AmrSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Controller 會以極快頻率 (例如 50Hz) 呼叫此函式
  // TODO: 透過 read() 從 Serial Port 讀取 ESP32 傳來的 JSON 或字串
  // 假設收到的字串為: "V,1.5,1.5\n" (代表左右輪速皆為 1.5 rad/s)
  
  // 這裡放入假資料作為測試，實際上應該解析 Serial 數據並更新 hw_states_
  // hw_states_[0] = left_wheel_velocity_from_esp32;
  // hw_states_[1] = right_wheel_velocity_from_esp32;

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type AmrSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // 將 ROS 2 導航演算法算出的目標輪速 (hw_commands_)，轉成字串發給 ESP32
  // 例如 hw_commands_[0] 是左輪目標，hw_commands_[1] 是右輪目標
  
  std::stringstream ss;
  ss << "CMD," << hw_commands_[0] << "," << hw_commands_[1] << "\n";
  std::string command_str = ss.str();

  // TODO: 透過 write() 將 command_str 發送進 Serial Port
  // RCLCPP_INFO(rclcpp::get_logger("AmrSystemHardware"), "發送指令: %s", command_str.c_str());

  return hardware_interface::return_type::OK;
}

}  // namespace amr_hardware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  amr_hardware::AmrSystemHardware, hardware_interface::SystemInterface)
