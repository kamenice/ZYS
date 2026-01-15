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

import socket from '@ohos.net.socket'

// MQTT配置 - 与wifi_mqtt.h保持一致，实际部署时应从配置文件读取
const MQTT_HOST: string = '10.0.2.2'
const MQTT_PORT: number = 1883
const MQTT_TOPIC_STATUS: string = 'conveyor/status'
const MQTT_TOPIC_CONTROL: string = 'conveyor/control'
const MQTT_TOPIC_ALERT: string = 'conveyor/alert'

// 传送带状态数据接口
interface ConveyorStatus {
  weight: number       // 重量 (克)
  temp: number         // 温度 (摄氏度)
  speed: number        // 速度 (RPM)
  running: number      // 运行状态 (0=停止, 1=运行)
  time: number         // 运行时间 (秒)
}

// 告警数据接口
interface AlertData {
  alert: string        // 告警类型
  value: number        // 告警值
}

// MQTT数据包类型
const MQTT_CONNECT: number = 0x10
const MQTT_CONNACK: number = 0x20
const MQTT_PUBLISH: number = 0x30
const MQTT_SUBSCRIBE: number = 0x82
const MQTT_SUBACK: number = 0x90
const MQTT_PINGREQ: number = 0xC0
const MQTT_PINGRESP: number = 0xD0
const MQTT_DISCONNECT: number = 0xE0

// 字符串转UTF-8字节数组 (处理BMP字符，支持中文)
function stringToUtf8Bytes(str: string): number[] {
  const bytes: number[] = []
  for (let i = 0; i < str.length; i++) {
    let code = str.charCodeAt(i)
    // 处理代理对 (surrogate pairs) - 4字节UTF-8
    if (code >= 0xD800 && code <= 0xDBFF && i + 1 < str.length) {
      const lowSurrogate = str.charCodeAt(i + 1)
      if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
        code = 0x10000 + ((code - 0xD800) << 10) + (lowSurrogate - 0xDC00)
        bytes.push(0xF0 | (code >> 18))
        bytes.push(0x80 | ((code >> 12) & 0x3F))
        bytes.push(0x80 | ((code >> 6) & 0x3F))
        bytes.push(0x80 | (code & 0x3F))
        i++ // 跳过低代理
        continue
      }
    }
    if (code < 0x80) {
      bytes.push(code)
    } else if (code < 0x800) {
      bytes.push(0xC0 | (code >> 6))
      bytes.push(0x80 | (code & 0x3F))
    } else if (code < 0x10000) {
      bytes.push(0xE0 | (code >> 12))
      bytes.push(0x80 | ((code >> 6) & 0x3F))
      bytes.push(0x80 | (code & 0x3F))
    }
  }
  return bytes
}

// UTF-8字节数组转字符串 (带边界检查)
function utf8BytesToString(bytes: Uint8Array, start: number, length: number): string {
  let result = ''
  let i = start
  const end = start + length
  while (i < end) {
    const byte1 = bytes[i]
    if (byte1 < 0x80) {
      result += String.fromCharCode(byte1)
      i++
    } else if ((byte1 & 0xE0) === 0xC0 && i + 1 < end) {
      const byte2 = bytes[i + 1]
      result += String.fromCharCode(((byte1 & 0x1F) << 6) | (byte2 & 0x3F))
      i += 2
    } else if ((byte1 & 0xF0) === 0xE0 && i + 2 < end) {
      const byte2 = bytes[i + 1]
      const byte3 = bytes[i + 2]
      result += String.fromCharCode(((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F))
      i += 3
    } else if ((byte1 & 0xF8) === 0xF0 && i + 3 < end) {
      // 4字节UTF-8 -> 代理对
      const byte2 = bytes[i + 1]
      const byte3 = bytes[i + 2]
      const byte4 = bytes[i + 3]
      const codePoint = ((byte1 & 0x07) << 18) | ((byte2 & 0x3F) << 12) | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F)
      const adjusted = codePoint - 0x10000
      result += String.fromCharCode(0xD800 + (adjusted >> 10), 0xDC00 + (adjusted & 0x3FF))
      i += 4
    } else {
      i++ // 跳过无效字节
    }
  }
  return result
}

@Entry
@Component
struct ConveyorMonitor {
  // 状态数据
  @State weight: number = 0
  @State temperature: number = 0
  @State speed: number = 0
  @State isRunning: boolean = false
  @State runTimeSeconds: number = 0

  // 告警状态
  @State isOverweight: boolean = false
  @State isOverheat: boolean = false
  @State isJammed: boolean = false
  @State lastAlert: string = ''

  // 连接状态
  @State isConnected: boolean = false
  @State connectionStatus: string = '未连接'

  // TCP Socket
  private tcpSocket: socket.TCPSocket | null = null
  private messageId: number = 1
  private keepAliveTimer: number = -1
  private isPingSending: boolean = false  // 防止心跳重叠

  aboutToAppear(): void {
    this.connectMqtt()
  }

  aboutToDisappear(): void {
    this.disconnectMqtt()
  }

  // 编码剩余长度 (MQTT协议)
  encodeRemainingLength(length: number): number[] {
    const bytes: number[] = []
    do {
      let byte = length % 128
      length = Math.floor(length / 128)
      if (length > 0) {
        byte = byte | 0x80
      }
      bytes.push(byte)
    } while (length > 0)
    return bytes
  }

  // 编码MQTT字符串 (长度前缀 + UTF-8字节)
  encodeString(str: string): number[] {
    const bytes = stringToUtf8Bytes(str)
    const len = bytes.length
    return [Math.floor(len / 256), len % 256, ...bytes]
  }

  // 构建CONNECT数据包
  buildConnectPacket(): ArrayBuffer {
    const clientId = 'arkts_' + Date.now().toString()
    const username = 'conveyor'
    const password = 'conveyor123'

    // 可变头部
    const protocolName = this.encodeString('MQTT')
    const protocolLevel = 0x04  // MQTT 3.1.1
    const connectFlags = 0xC2   // Username, Password, Clean Session
    const keepAlive = [0x00, 0x3C]  // 60秒

    // 有效载荷
    const clientIdBytes = this.encodeString(clientId)
    const usernameBytes = this.encodeString(username)
    const passwordBytes = this.encodeString(password)

    const variableHeader = [...protocolName, protocolLevel, connectFlags, ...keepAlive]
    const payload = [...clientIdBytes, ...usernameBytes, ...passwordBytes]
    const remainingLength = this.encodeRemainingLength(variableHeader.length + payload.length)

    const packet = new Uint8Array([MQTT_CONNECT, ...remainingLength, ...variableHeader, ...payload])
    return packet.buffer
  }

  // 构建SUBSCRIBE数据包
  buildSubscribePacket(topic: string): ArrayBuffer {
    const packetId = [Math.floor(this.messageId / 256), this.messageId % 256]
    this.messageId++

    const topicBytes = this.encodeString(topic)
    const qos = 0x00

    const payload = [...topicBytes, qos]
    const remainingLength = this.encodeRemainingLength(2 + payload.length)

    const packet = new Uint8Array([MQTT_SUBSCRIBE, ...remainingLength, ...packetId, ...payload])
    return packet.buffer
  }

  // 构建PUBLISH数据包
  buildPublishPacket(topic: string, message: string): ArrayBuffer {
    const topicBytes = this.encodeString(topic)
    const messageBytes = stringToUtf8Bytes(message)

    const remainingLength = this.encodeRemainingLength(topicBytes.length + messageBytes.length)

    const packet = new Uint8Array([MQTT_PUBLISH, ...remainingLength, ...topicBytes, ...messageBytes])
    return packet.buffer
  }

  // 构建PINGREQ数据包
  buildPingPacket(): ArrayBuffer {
    return new Uint8Array([MQTT_PINGREQ, 0x00]).buffer
  }

  // 构建DISCONNECT数据包
  buildDisconnectPacket(): ArrayBuffer {
    return new Uint8Array([MQTT_DISCONNECT, 0x00]).buffer
  }

  // 解析PUBLISH消息
  parsePublishMessage(data: Uint8Array, startIndex: number, remainingLength: number): void {
    try {
      // 读取主题长度
      const topicLen = (data[startIndex] << 8) | data[startIndex + 1]
      const topic = utf8BytesToString(data, startIndex + 2, topicLen)

      // 读取消息内容
      const messageStart = startIndex + 2 + topicLen
      const messageLen = remainingLength - 2 - topicLen
      const message = utf8BytesToString(data, messageStart, messageLen)

      this.handleMessage(topic, message)
    } catch (error) {
      console.error('解析PUBLISH消息错误:', error)
    }
  }

  // 处理接收到的数据
  handleReceivedData(data: ArrayBuffer): void {
    const bytes = new Uint8Array(data)

    let index = 0
    while (index < bytes.length) {
      const packetType = bytes[index] & 0xF0

      // 解析剩余长度
      let multiplier = 1
      let remainingLength = 0
      let lengthIndex = index + 1
      let encodedByte: number
      do {
        if (lengthIndex >= bytes.length) {
          // 数据不完整，等待更多数据
          console.info('等待更多数据包...')
          return
        }
        encodedByte = bytes[lengthIndex]
        remainingLength += (encodedByte & 127) * multiplier
        multiplier *= 128
        lengthIndex++
      } while ((encodedByte & 128) !== 0)

      const headerLength = lengthIndex - index
      const totalLength = headerLength + remainingLength

      if (index + totalLength > bytes.length) {
        // 数据包不完整，等待更多数据
        console.info('数据包不完整，等待更多数据...')
        return
      }

      switch (packetType) {
        case MQTT_CONNACK:
          if (remainingLength >= 2 && bytes[index + headerLength + 1] === 0) {
            this.isConnected = true
            this.connectionStatus = '已连接'
            // 订阅主题
            this.subscribe(MQTT_TOPIC_STATUS)
            this.subscribe(MQTT_TOPIC_ALERT)
            // 启动心跳
            this.startKeepAlive()
          } else {
            this.connectionStatus = '连接被拒绝'
          }
          break
        case MQTT_PUBLISH:
          this.parsePublishMessage(bytes, index + headerLength, remainingLength)
          break
        case MQTT_SUBACK:
          console.info('订阅确认')
          break
        case MQTT_PINGRESP:
          console.info('心跳响应')
          break
      }

      index += totalLength
    }
  }

  // 连接MQTT服务器
  async connectMqtt(): Promise<void> {
    try {
      this.connectionStatus = '连接中...'
      this.tcpSocket = socket.constructTCPSocketInstance()

      // 监听消息
      this.tcpSocket.on('message', (value: socket.SocketMessageInfo) => {
        this.handleReceivedData(value.message)
      })

      // 监听连接状态
      this.tcpSocket.on('close', () => {
        this.isConnected = false
        this.connectionStatus = '连接已断开'
        this.stopKeepAlive()
      })

      // 监听错误
      this.tcpSocket.on('error', (err: Error) => {
        this.isConnected = false
        this.connectionStatus = '连接错误'
        console.error('Socket错误:', err)
      })

      // 连接服务器
      await this.tcpSocket.connect({
        address: {
          address: MQTT_HOST,
          port: MQTT_PORT,
          family: 1  // IPv4
        },
        timeout: 10000
      })

      // 发送CONNECT数据包
      const connectPacket = this.buildConnectPacket()
      await this.tcpSocket.send({ data: connectPacket })

    } catch (error) {
      this.connectionStatus = '连接失败'
      console.error('连接MQTT失败:', error)
    }
  }

  // 订阅主题
  async subscribe(topic: string): Promise<void> {
    if (this.tcpSocket && this.isConnected) {
      try {
        const packet = this.buildSubscribePacket(topic)
        await this.tcpSocket.send({ data: packet })
      } catch (error) {
        console.error('订阅失败:', error)
      }
    }
  }

  // 发送控制命令
  async sendCommand(command: string): Promise<void> {
    if (this.tcpSocket && this.isConnected) {
      try {
        const packet = this.buildPublishPacket(MQTT_TOPIC_CONTROL, command)
        await this.tcpSocket.send({ data: packet })
      } catch (error) {
        console.error('发送命令失败:', error)
      }
    }
  }

  // 启动心跳定时器
  startKeepAlive(): void {
    this.keepAliveTimer = setInterval(async () => {
      // 防止心跳重叠发送
      if (this.isPingSending) {
        return
      }
      if (this.tcpSocket && this.isConnected) {
        this.isPingSending = true
        try {
          const packet = this.buildPingPacket()
          await this.tcpSocket.send({ data: packet })
        } catch (error) {
          console.error('心跳发送失败:', error)
        } finally {
          this.isPingSending = false
        }
      }
    }, 30000)  // 30秒发送一次心跳
  }

  // 停止心跳定时器
  stopKeepAlive(): void {
    if (this.keepAliveTimer !== -1) {
      clearInterval(this.keepAliveTimer)
      this.keepAliveTimer = -1
    }
    this.isPingSending = false
  }

  // 断开MQTT连接
  async disconnectMqtt(): Promise<void> {
    this.stopKeepAlive()
    if (this.tcpSocket) {
      try {
        if (this.isConnected) {
          const packet = this.buildDisconnectPacket()
          await this.tcpSocket.send({ data: packet })
        }
        await this.tcpSocket.close()
      } catch (error) {
        console.error('断开连接错误:', error)
      }
      this.tcpSocket = null
    }
    this.isConnected = false
  }

  // 处理MQTT消息
  handleMessage(topic: string, message: string): void {
    try {
      if (topic === MQTT_TOPIC_STATUS) {
        const status: ConveyorStatus = JSON.parse(message)
        this.weight = status.weight
        this.temperature = status.temp
        this.speed = status.speed
        this.isRunning = status.running === 1
        this.runTimeSeconds = status.time
        // 清除之前的告警状态
        this.isOverweight = false
        this.isOverheat = false
        this.isJammed = false
      } else if (topic === MQTT_TOPIC_ALERT) {
        const alert: AlertData = JSON.parse(message)
        this.handleAlert(alert)
      }
    } catch (error) {
      console.error('消息解析错误:', error)
    }
  }

  // 处理告警消息
  handleAlert(alert: AlertData): void {
    switch (alert.alert) {
      case 'overweight':
        this.isOverweight = true
        this.lastAlert = '超重告警: ' + alert.value.toFixed(1) + '克'
        break
      case 'overheat':
        this.isOverheat = true
        this.lastAlert = '过热告警: ' + alert.value.toFixed(1) + '°C'
        break
      case 'jam':
        this.isJammed = true
        this.lastAlert = '堵塞告警'
        break
    }
  }

  // 格式化运行时间
  formatTime(seconds: number): string {
    const mins: number = Math.floor(seconds / 60)
    const secs: number = seconds % 60
    return mins.toString().padStart(2, '0') + ':' + secs.toString().padStart(2, '0')
  }

  build() {
    Column() {
      // 标题栏
      Row() {
        Text('传送带智能监控系统')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor('#FFFFFF')
      }
      .width('100%')
      .height(60)
      .backgroundColor('#1976D2')
      .justifyContent(FlexAlign.Center)

      // 连接状态
      Row() {
        Circle()
          .width(12)
          .height(12)
          .fill(this.isConnected ? '#4CAF50' : '#F44336')
        Text(this.connectionStatus)
          .fontSize(14)
          .margin({ left: 8 })
          .fontColor(this.isConnected ? '#4CAF50' : '#F44336')
      }
      .width('100%')
      .padding(10)
      .justifyContent(FlexAlign.Center)

      // 数据显示区域
      Column() {
        // 运行状态
        Row() {
          Text('运行状态')
            .fontSize(16)
            .fontColor('#666666')
          Blank()
          Text(this.isRunning ? '运行中' : '已停止')
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(this.isRunning ? '#4CAF50' : '#F44336')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFFFFF')
        .borderRadius(8)
        .margin({ bottom: 10 })

        // 重量显示
        Row() {
          Text('当前重量')
            .fontSize(16)
            .fontColor('#666666')
          Blank()
          Text(this.weight.toFixed(1) + ' 克')
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(this.isOverweight ? '#F44336' : '#333333')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFFFFF')
        .borderRadius(8)
        .margin({ bottom: 10 })

        // 温度显示
        Row() {
          Text('当前温度')
            .fontSize(16)
            .fontColor('#666666')
          Blank()
          Text(this.temperature.toFixed(1) + ' °C')
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(this.isOverheat ? '#F44336' : '#333333')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFFFFF')
        .borderRadius(8)
        .margin({ bottom: 10 })

        // 速度显示
        Row() {
          Text('转速')
            .fontSize(16)
            .fontColor('#666666')
          Blank()
          Text(this.speed.toFixed(0) + ' RPM')
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFFFFF')
        .borderRadius(8)
        .margin({ bottom: 10 })

        // 运行时间显示
        Row() {
          Text('运行时间')
            .fontSize(16)
            .fontColor('#666666')
          Blank()
          Text(this.formatTime(this.runTimeSeconds))
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor('#333333')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFFFFF')
        .borderRadius(8)
        .margin({ bottom: 10 })
      }
      .width('100%')
      .padding(15)

      // 告警显示
      if (this.lastAlert !== '') {
        Row() {
          Text('⚠ ' + this.lastAlert)
            .fontSize(14)
            .fontColor('#FFFFFF')
        }
        .width('90%')
        .padding(10)
        .backgroundColor('#FF5722')
        .borderRadius(8)
        .justifyContent(FlexAlign.Center)
        .margin({ bottom: 15 })
      }

      // 控制按钮区域
      Column() {
        Text('控制面板')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .margin({ bottom: 15 })

        Row() {
          // 启动按钮
          Button('启动')
            .width('45%')
            .height(50)
            .fontSize(16)
            .backgroundColor('#4CAF50')
            .onClick(() => {
              this.sendCommand('start')
            })

          // 停止按钮
          Button('停止')
            .width('45%')
            .height(50)
            .fontSize(16)
            .backgroundColor('#F44336')
            .onClick(() => {
              this.sendCommand('stop')
            })
        }
        .width('100%')
        .justifyContent(FlexAlign.SpaceBetween)
        .margin({ bottom: 10 })

        Row() {
          // 静音按钮
          Button('静音')
            .width('45%')
            .height(50)
            .fontSize(16)
            .backgroundColor('#FF9800')
            .onClick(() => {
              this.sendCommand('mute')
            })

          // 取消静音按钮
          Button('取消静音')
            .width('45%')
            .height(50)
            .fontSize(16)
            .backgroundColor('#2196F3')
            .onClick(() => {
              this.sendCommand('unmute')
            })
        }
        .width('100%')
        .justifyContent(FlexAlign.SpaceBetween)
        .margin({ bottom: 10 })

        // 清除堵塞按钮
        Button('清除堵塞')
          .width('100%')
          .height(50)
          .fontSize(16)
          .backgroundColor('#9C27B0')
          .onClick(() => {
            this.sendCommand('clear_jam')
          })
      }
      .width('100%')
      .padding(15)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }
}