# 传送带智能系统 (Conveyor Belt Intelligent System)

基于Hi3861芯片的智能传送带控制系统。

## 功能特性

### 1. 启动识别功能
- **传感器**: HX711压力传感器
- **功能**: 当检测到物品放置在传送带上时，自动启动传送带
- **控制方式**: 
  - 按键手动启动/停止
  - APP远程一键启动/停止

### 2. 防堆积卡件功能
- **传感器**: YL-62红外避障传感器
- **功能**: 当检测到物品堆积卡件时，自动启动SG90舵机振动传送带实现抖动出件
- **控制方式**: 可通过APP手动触发出件

### 3. 防重物功能
- **传感器**: HX711压力传感器
- **功能**: 当物品超过设定重量阈值时：
  - 持续启动蜂鸣器报警
  - 通过MQTT向APP推送超重警报

### 4. 防传送带过热功能
- **传感器**: DHT11温湿度传感器
- **功能**: 监控轴承与传送带之间的温度，当温度超过阈值时：
  - 自动停止传送带
  - 间歇启动蜂鸣器提醒维护
  - 通过MQTT向APP推送温度过高警报

### 5. 测量转速功能
- **传感器**: 3411霍尔效应传感器
- **功能**: 通过检测转轴上磁铁经过传感器的信号，实时测量传送带转速

### 6. 状态显示功能
- **显示器**: OLED屏幕
- **显示内容**:
  - 当前物品重量
  - 是否卡件/超重
  - 传送带运行状态
  - 轴承温度
  - 传送带速度
  - 运行时间

### 7. 远程控制功能
- **通信**: WiFi + MQTT协议
- **功能**:
  - 远程监控传送带状态、温度、速度、重量、运行时间
  - 设置超重和温度报警阈值
  - 接收报警通知推送
  - 一键静音蜂鸣器
  - 远程启动/停止传送带

## 硬件配置

| 组件 | 型号 | GPIO引脚 |
|------|------|----------|
| 压力传感器 | HX711 | DT: GPIO4*, SCK: GPIO12 |
| 温湿度传感器 | DHT11 | GPIO7 |
| 红外避障传感器 | YL-62 | GPIO8 |
| 霍尔传感器 | 3411 | GPIO11 |
| 舵机 | SG90 | GPIO5 (PWM3) |
| 电机驱动 | TB6612FNG | PWMA: GPIO10, AIN1: GPIO6, AIN2: GPIO1, STBY: 接VCC |
| 蜂鸣器(有源低电平) | - | GPIO9 |
| OLED显示屏 | SSD1306 | I2C0 (SDA: GPIO13, SCL: GPIO14) |

**注意**: 
- GPIO0、GPIO2、GPIO3、GPIO4 与系统调试串口(UART0)相关，可能存在冲突
- GPIO4用于HX711，可能会干扰串口接收功能
- GPIO3用户按键功能已禁用（与UART0_TXD冲突），请使用MQTT远程控制
- TB6612FNG的STBY引脚需要在硬件上连接到VCC以保持常开状态

## MQTT主题

| 主题 | 方向 | 说明 |
|------|------|------|
| conveyor/status | 发布 | 状态信息 (JSON格式) |
| conveyor/control | 订阅 | 控制命令 |
| conveyor/alert | 发布 | 报警信息 |
| conveyor/config | 订阅 | 配置更新 |

### 控制命令
- `start` - 启动传送带
- `stop` - 停止传送带
- `mute` - 静音蜂鸣器
- `unmute` - 取消静音
- `clear_jam` - 清除卡件
- `set_weight <value>` - 设置重量阈值
- `set_temp <value>` - 设置温度阈值

### 状态消息格式
```json
{
  "weight": 1234.5,
  "temp": 45.2,
  "speed": 120.0,
  "running": 1,
  "time": 3600
}
```

### 警报消息格式
```json
{
  "alert": "overweight",
  "value": 6000.0
}
```

## 配置说明

### WiFi配置
修改 `wifi_mqtt.h` 中的以下定义:
```c
#define WIFI_SSID               "your_wifi_ssid"
#define WIFI_PASSWORD           "your_wifi_password"
```

### MQTT配置
修改 `wifi_mqtt.h` 中的以下定义:
```c
#define MQTT_HOST               "192.168.1.100"
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          "hi3861_conveyor"
#define MQTT_USERNAME           "conveyor"
#define MQTT_PASSWORD           "conveyor123"
```

### 阈值配置
默认阈值在 `conveyor_system.h` 中定义:
```c
#define DEFAULT_WEIGHT_THRESHOLD    5000.0  // 5kg
#define DEFAULT_TEMP_THRESHOLD      60.0f   // 60°C
```

## 编译说明

1. 在 `applications/sample/wifi-iot/app/BUILD.gn` 中启用conveyor_belt组件:
```gn
lite_component("app") {
    features = [
        "code/conveyor_belt:conveyor_belt",
    ]
}
```

2. 执行编译:
```bash
python build.py wifiiot
```

## 文件结构

```
code/conveyor_belt/
├── BUILD.gn              # 构建配置
├── README.md             # 项目说明
├── main.c                # 主程序入口
├── conveyor_system.c/h   # 系统控制逻辑
├── hx711.c/h             # HX711压力传感器驱动
├── dht11.c/h             # DHT11温湿度传感器驱动
├── infrared.c/h          # YL-62红外传感器驱动
├── hall_sensor.c/h       # 3411霍尔传感器驱动
├── motor.c/h             # TB6612FNG电机驱动
├── servo.c/h             # SG90舵机驱动
├── buzzer.c/h            # 蜂鸣器驱动
├── oled_display.c/h      # OLED显示驱动
└── wifi_mqtt.c/h         # WiFi和MQTT通信模块
```

## 许可证

Apache License 2.0
