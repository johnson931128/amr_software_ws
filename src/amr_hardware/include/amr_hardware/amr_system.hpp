#ifndef AMR_HARDWARE__AMR_SYSTEM_HPP_
#define AMR_HARDWARE__AMR_SYSTEM_HPP_

#include <vector>
#include <string>
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace amr_hardware
{
class AmrSystemHardware : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(AmrSystemHardware)

  // 1. 初始化硬體參數 (自 URDF 解析設定)
  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareInfo & info) override;

  // 2. 匯出狀態介面 (例如：提供目前輪速給系統)
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  // 3. 匯出指令介面 (例如：開放目標輪速寫入權限)
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  // 4. 啟動硬體連線 (例如：開啟 Serial Port)
  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  // 5. 關閉硬體連線 (例如：關閉 Serial Port)
  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  // 6. 讀取數據 (自 ESP32 接收感測器資料)
  hardware_interface::return_type read(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  // 7. 寫入數據 (將目標速度指令發送至 ESP32)
  hardware_interface::return_type write(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  // 內部變數宣告，用於暫存通訊數據
  std::vector<double> hw_commands_; // 儲存預備下達給馬達的目標速度
  std::vector<double> hw_states_;   // 儲存自編碼器讀取之實際輪速
};

}  // namespace amr_hardware

#endif  // AMR_HARDWARE__AMR_SYSTEM_HPP_
