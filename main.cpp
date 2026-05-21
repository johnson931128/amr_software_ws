#include <Arduino.h>
#include <Wire.h>

// --- micro-ROS 相關函式庫 ---
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <geometry_msgs/msg/twist.h>
#include <nav_msgs/msg/odometry.h>

// ==========================================
// 1. 硬體參數與腳位設定
// ==========================================
const int I2C_SDA = 21, I2C_SCL = 22;

const int ENC_L_A = 32, ENC_L_B = 33;
const int ENC_R_A = 25, ENC_R_B = 26;

const int ENA = 23, IN1 = 19, IN2 = 18;
const int ENB = 4,  IN3 = 17, IN4 = 16;
const int pwmChannelL = 0, pwmChannelR = 1;

//  AMR 神聖三大參數 (親手測量的)
const float WHEEL_RADIUS = 0.035; // 主動輪等效半徑 (m)
const float TPR = 1056.7;         // 履帶馬達轉一圈的脈衝數
const float L = 0.325;            // 履帶中心距 (m)
const int DEADZONE = 150;         // 馬達啟動最低 PWM

// ==========================================
// 2. 系統變數與 PID 參數
// ==========================================
volatile long count_L = 0, count_R = 0;
long prev_count_L = 0, prev_count_R = 0;

float target_speed_L = 0.0; 
float target_speed_R = 0.0; 

float Kp = 300.0, Ki = 15.0, Kd = 5.0;
float integral_L = 0, prev_error_L = 0;
float integral_R = 0, prev_error_R = 0;

unsigned long prev_time = 0;
float gyro_z_offset = 0;

// 里程計 (Odometry) 累積變數
float odom_x = 0.0;
float odom_y = 0.0;
float odom_theta = 0.0;

// --- micro-ROS 物件 ---
rcl_publisher_t odom_publisher;
rcl_subscription_t cmd_vel_subscriber;
geometry_msgs__msg__Twist cmd_vel_msg;
nav_msgs__msg__Odometry odom_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// ==========================================
// 3. 中斷與硬體操作函式
// ==========================================
void IRAM_ATTR isr_enc_L() {
  if (digitalRead(ENC_L_A) == digitalRead(ENC_L_B)) count_L++; else count_L--;
}
void IRAM_ATTR isr_enc_R() {
  if (digitalRead(ENC_R_A) == digitalRead(ENC_R_B)) count_R++; else count_R--;
}

void setMotorPWM(int channel, int inA, int inB, int pwm_val) {
  if (pwm_val > 0) {
    digitalWrite(inA, HIGH); digitalWrite(inB, LOW);
  } else if (pwm_val < 0) {
    digitalWrite(inA, LOW); digitalWrite(inB, HIGH);
    pwm_val = -pwm_val;
  } else {
    digitalWrite(inA, LOW); digitalWrite(inB, LOW);
  }
  if (pwm_val > 255) pwm_val = 255;
  ledcWrite(channel, pwm_val);
}

float readGyroZ() {
  Wire.beginTransmission(0x68); Wire.write(0x47); Wire.endTransmission(false);
  Wire.requestFrom(0x68, 2, true);
  int16_t rawZ = (Wire.read() << 8 | Wire.read());
  return (rawZ - gyro_z_offset) / 131.0 * (PI / 180.0); // 轉換為 rad/s
}

// ==========================================
// 4. ROS 2 回呼函式 (接收 /cmd_vel)
// ==========================================
// 當大腦傳送速度指令時，這個函式會被觸發，利用運動學將整體速度拆解給左右輪
void cmd_vel_callback(const void * msgin) {
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
  float linear_x = msg->linear.x;
  float angular_z = msg->angular.z;

  // 差速輪運動學逆解
  target_speed_L = linear_x - (angular_z * L / 2.0);
  target_speed_R = linear_x + (angular_z * L / 2.0);
}

// ==========================================
// 5. 初始化 Setup
// ==========================================
void setup() {
  // 1. 初始化序列埠 (交給 micro-ROS 使用)
  Serial.begin(115200);
  set_microros_transports();
  delay(2000);

  // 2. 初始化感測器與馬達
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(0x68); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission();
  
  // 校正 MPU6500 (靜止 2 秒鐘)
  long offset_sum = 0;
  for(int i=0; i<200; i++) {
    Wire.beginTransmission(0x68); Wire.write(0x47); Wire.endTransmission(false);
    Wire.requestFrom(0x68, 2, true);
    offset_sum += (Wire.read() << 8 | Wire.read());
    delay(10);
  }
  gyro_z_offset = offset_sum / 200.0;

  pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
  pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), isr_enc_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), isr_enc_R, CHANGE);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  ledcSetup(pwmChannelL, 5000, 8); ledcAttachPin(ENA, pwmChannelL);
  ledcSetup(pwmChannelR, 5000, 8); ledcAttachPin(ENB, pwmChannelR);

  // 3. 初始化 micro-ROS 節點
  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "amr_esp32_base", "", &support);

  // 建立 Publisher (/odom)
  rclc_publisher_init_default(
    &odom_publisher, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
    "odom"
  );

  // 建立 Subscriber (/cmd_vel)
  rclc_subscription_init_default(
    &cmd_vel_subscriber, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel"
  );

  // 初始化執行器
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &cmd_vel_subscriber, &cmd_vel_msg, &cmd_vel_callback, ON_NEW_DATA);

  // 設定 Odom 字串記憶體 (防止崩潰)
  static char frame_id[] = "odom";
  static char child_frame_id[] = "base_link";
  odom_msg.header.frame_id.data = frame_id;
  odom_msg.header.frame_id.size = strlen(frame_id);
  odom_msg.child_frame_id.data = child_frame_id;
  odom_msg.child_frame_id.size = strlen(child_frame_id);

  prev_time = millis();
}

// ==========================================
// 6. 主迴圈 (ROS 處理 + PID 控制 + 里程計推算)
// ==========================================
void loop() {
  // 讓 micro-ROS 處理通訊 (接收 /cmd_vel)
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));

  unsigned long current_time = millis();
  float dt = (current_time - prev_time) / 1000.0;

  if (dt >= 0.05) { // 50ms 運算週期
    // --- 1. 計算實際輪速 ---
    long cur_count_L = count_L;
    long cur_count_R = count_R;
    float speed_L = ((cur_count_L - prev_count_L) / TPR) * (2 * PI * WHEEL_RADIUS) / dt;
    float speed_R = ((cur_count_R - prev_count_R) / TPR) * (2 * PI * WHEEL_RADIUS) / dt;
    prev_count_L = cur_count_L;
    prev_count_R = cur_count_R;

    // --- 2. PID 計算與死區補償 ---
    int final_pwm_L = 0, final_pwm_R = 0;

    // 左輪
    if (abs(target_speed_L) < 0.01) { // 如果目標是 0，強制煞車，防止高頻嘯叫
      final_pwm_L = 0; integral_L = 0; prev_error_L = 0;
    } else {
      float error_L = target_speed_L - speed_L;
      integral_L += error_L * dt;
      float deriv_L = (error_L - prev_error_L) / dt;
      float pid_L = (Kp * error_L) + (Ki * integral_L) + (Kd * deriv_L);
      final_pwm_L = (target_speed_L > 0) ? (pid_L + DEADZONE) : (pid_L - DEADZONE);
      prev_error_L = error_L;
    }

    // 右輪
    if (abs(target_speed_R) < 0.01) { 
      final_pwm_R = 0; integral_R = 0; prev_error_R = 0;
    } else {
      float error_R = target_speed_R - speed_R;
      integral_R += error_R * dt;
      float deriv_R = (error_R - prev_error_R) / dt;
      float pid_R = (Kp * error_R) + (Ki * integral_R) + (Kd * deriv_R);
      final_pwm_R = (target_speed_R > 0) ? (pid_R + DEADZONE) : (pid_R - DEADZONE);
      prev_error_R = error_R;
    }

    setMotorPWM(pwmChannelL, IN1, IN2, final_pwm_L);
    setMotorPWM(pwmChannelR, IN3, IN4, final_pwm_R);

    // --- 3. 運動學正向解：計算里程計 (Odometry) ---
    float v_linear = (speed_R + speed_L) / 2.0; 
    float v_angular = readGyroZ(); // 直接採用 MPU6500 的數值，比輪子算出來的更準確防滑！

    // 更新座標
    odom_theta += v_angular * dt;
    odom_x += v_linear * cos(odom_theta) * dt;
    odom_y += v_linear * sin(odom_theta) * dt;

    // --- 4. 發布 /odom 訊息 ---
    odom_msg.header.stamp.sec = current_time / 1000;
    odom_msg.header.stamp.nanosec = (current_time % 1000) * 1000000;
    
    odom_msg.pose.pose.position.x = odom_x;
    odom_msg.pose.pose.position.y = odom_y;
    // 簡化的 Quaternion 轉換 (只在 Z 軸旋轉)
    odom_msg.pose.pose.orientation.z = sin(odom_theta / 2.0);
    odom_msg.pose.pose.orientation.w = cos(odom_theta / 2.0);
    
    odom_msg.twist.twist.linear.x = v_linear;
    odom_msg.twist.twist.angular.z = v_angular;

    rcl_publish(&odom_publisher, &odom_msg, NULL);

    prev_time = current_time;
  }
}