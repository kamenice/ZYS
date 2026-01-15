/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Smart Conveyor Belt System - HarmonyOS Mobile App Interface
 *
 * This ArkTS UI file is designed for DevEco Studio to create a mobile app
 * that monitors and displays the conveyor belt system status via MQTT.
 *
 * MQTT Configuration:
 * - WiFi: SSID "zys", Password "12345678"
 * - Server: 192.168.43.230:1883
 * - Topic: "conveyor"
 * - Data Format: {"hasItem":X,"temperature":Y.Y,"motorState":Z}
 *
 * Usage:
 * 1. Create a new HarmonyOS project in DevEco Studio
 * 2. Replace the default Index.ets with this file
 * 3. Configure MQTT connection in the ability
 */

@Entry
@Component
struct Index {
  // State variables for conveyor belt status
  @State hasItem: boolean = false           // Item detected by pressure sensor
  @State temperature: number = 0.0          // Temperature from DHT11 sensor
  @State motorState: number = 0             // Motor state: 0=stopped, 1=running
  @State isConnected: boolean = false       // MQTT connection status
  @State lastUpdateTime: string = '--:--:--' // Last data update time

  // Temperature threshold for alarm
  private readonly TEMP_THRESHOLD: number = 30.0

  // Simulate MQTT data reception (in real app, connect to MQTT broker)
  aboutToAppear() {
    // Initialize connection status
    this.isConnected = false

    // In a real app, initialize MQTT client here:
    // this.initMqttClient('192.168.43.230', 1883, 'conveyor')
  }

  // Parse incoming MQTT JSON data
  parseData(jsonStr: string): void {
    try {
      const data = JSON.parse(jsonStr)
      this.hasItem = data.hasItem === 1
      this.temperature = data.temperature
      this.motorState = data.motorState
      this.updateTime()
    } catch (e) {
      console.error('Failed to parse MQTT data')
    }
  }

  // Update last update time
  updateTime(): void {
    const now = new Date()
    this.lastUpdateTime = `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`
  }

  // Get motor status text
  getMotorStatusText(): string {
    return this.motorState === 1 ? '运行中' : '已停止'
  }

  // Get motor status color
  getMotorStatusColor(): string {
    return this.motorState === 1 ? '#4CAF50' : '#9E9E9E'
  }

  // Get item status text
  getItemStatusText(): string {
    return this.hasItem ? '有物品' : '无物品'
  }

  // Get item status color
  getItemStatusColor(): string {
    return this.hasItem ? '#2196F3' : '#9E9E9E'
  }

  // Get temperature status color
  getTempStatusColor(): string {
    return this.temperature >= this.TEMP_THRESHOLD ? '#F44336' : '#4CAF50'
  }

  // Check if temperature alarm is active
  isAlarmActive(): boolean {
    return this.temperature >= this.TEMP_THRESHOLD
  }

  build() {
    Column() {
      // Header
      Row() {
        Text('智能传送带系统')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor('#333333')
      }
      .width('100%')
      .height(60)
      .justifyContent(FlexAlign.Center)
      .backgroundColor('#FFFFFF')

      // Connection status bar
      Row() {
        Circle()
          .width(12)
          .height(12)
          .fill(this.isConnected ? '#4CAF50' : '#F44336')
        Text(this.isConnected ? '已连接' : '未连接')
          .fontSize(14)
          .fontColor(this.isConnected ? '#4CAF50' : '#F44336')
          .margin({ left: 8 })
        Blank()
        Text(`更新时间: ${this.lastUpdateTime}`)
          .fontSize(12)
          .fontColor('#666666')
      }
      .width('100%')
      .padding({ left: 16, right: 16, top: 8, bottom: 8 })
      .backgroundColor('#F5F5F5')

      // Main content area
      Column() {
        // Item detection card
        Column() {
          Row() {
            Image($r('app.media.ic_item'))
              .width(40)
              .height(40)
              .objectFit(ImageFit.Contain)
            Column() {
              Text('物品检测')
                .fontSize(14)
                .fontColor('#666666')
              Text(this.getItemStatusText())
                .fontSize(20)
                .fontWeight(FontWeight.Bold)
                .fontColor(this.getItemStatusColor())
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 16 })
          }
          .width('100%')
          .padding(16)
        }
        .width('100%')
        .backgroundColor('#FFFFFF')
        .borderRadius(12)
        .margin({ bottom: 12 })

        // Temperature card
        Column() {
          Row() {
            Image($r('app.media.ic_temp'))
              .width(40)
              .height(40)
              .objectFit(ImageFit.Contain)
            Column() {
              Text('当前温度')
                .fontSize(14)
                .fontColor('#666666')
              Row() {
                Text(`${this.temperature.toFixed(1)}`)
                  .fontSize(32)
                  .fontWeight(FontWeight.Bold)
                  .fontColor(this.getTempStatusColor())
                Text('°C')
                  .fontSize(16)
                  .fontColor(this.getTempStatusColor())
                  .margin({ left: 4, top: 8 })
              }
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 16 })
            Blank()
            if (this.isAlarmActive()) {
              Column() {
                Image($r('app.media.ic_alarm'))
                  .width(32)
                  .height(32)
                Text('高温警报!')
                  .fontSize(12)
                  .fontColor('#F44336')
              }
              .alignItems(HorizontalAlign.Center)
            }
          }
          .width('100%')
          .padding(16)

          // Temperature threshold indicator
          Row() {
            Text(`警报阈值: ${this.TEMP_THRESHOLD}°C`)
              .fontSize(12)
              .fontColor('#999999')
          }
          .width('100%')
          .padding({ left: 16, right: 16, bottom: 12 })
        }
        .width('100%')
        .backgroundColor('#FFFFFF')
        .borderRadius(12)
        .margin({ bottom: 12 })

        // Motor status card
        Column() {
          Row() {
            Image($r('app.media.ic_motor'))
              .width(40)
              .height(40)
              .objectFit(ImageFit.Contain)
            Column() {
              Text('电机状态')
                .fontSize(14)
                .fontColor('#666666')
              Text(this.getMotorStatusText())
                .fontSize(20)
                .fontWeight(FontWeight.Bold)
                .fontColor(this.getMotorStatusColor())
            }
            .alignItems(HorizontalAlign.Start)
            .margin({ left: 16 })
            Blank()
            // Motor indicator animation
            if (this.motorState === 1) {
              LoadingProgress()
                .width(40)
                .height(40)
                .color('#4CAF50')
            }
          }
          .width('100%')
          .padding(16)
        }
        .width('100%')
        .backgroundColor('#FFFFFF')
        .borderRadius(12)
        .margin({ bottom: 12 })

        // System info card
        Column() {
          Text('系统信息')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .fontColor('#333333')
            .width('100%')
            .padding({ left: 16, top: 12, bottom: 8 })

          Divider()
            .width('90%')
            .color('#EEEEEE')

          Row() {
            Text('WiFi名称')
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text('zys')
              .fontSize(14)
              .fontColor('#333333')
          }
          .width('100%')
          .padding({ left: 16, right: 16, top: 8, bottom: 8 })

          Row() {
            Text('服务器地址')
              .fontSize(14)
              .fontColor('#666666')
            Blank()
            Text('192.168.43.230:1883')
              .fontSize(14)
              .fontColor('#333333')
          }
          .width('100%')
          .padding({ left: 16, right: 16, top: 8, bottom: 12 })
        }
        .width('100%')
        .backgroundColor('#FFFFFF')
        .borderRadius(12)
      }
      .width('100%')
      .padding(16)
      .layoutWeight(1)

      // Footer with connect button
      Row() {
        Button(this.isConnected ? '断开连接' : '连接设备')
          .width('80%')
          .height(48)
          .fontSize(16)
          .fontColor('#FFFFFF')
          .backgroundColor(this.isConnected ? '#F44336' : '#2196F3')
          .borderRadius(24)
          .onClick(() => {
            this.isConnected = !this.isConnected
            if (this.isConnected) {
              // Simulate connection and data
              // In real app: connect to MQTT broker
              this.simulateDataUpdate()
            }
          })
      }
      .width('100%')
      .height(80)
      .justifyContent(FlexAlign.Center)
      .backgroundColor('#FFFFFF')
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F0F0F0')
  }

  // Simulate data updates for testing
  // In real app, this would be replaced by MQTT message handler
  simulateDataUpdate(): void {
    // Simulate initial data
    this.hasItem = true
    this.temperature = 25.5
    this.motorState = 1
    this.updateTime()

    // Set up periodic updates for demo
    setInterval(() => {
      if (this.isConnected) {
        // Simulate random data changes
        this.hasItem = Math.random() > 0.3
        this.temperature = 20 + Math.random() * 15 // 20-35°C
        this.motorState = this.hasItem && this.temperature < this.TEMP_THRESHOLD ? 1 : 0
        this.updateTime()
      }
    }, 2000)
  }
}
