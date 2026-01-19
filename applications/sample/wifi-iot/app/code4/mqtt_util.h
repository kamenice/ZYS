/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - MQTT Utility
 * 
 * MQTT for remote monitoring and control:
 * - Publish: Sensor data (weight, temperature, status)
 * - Subscribe: Control commands (start, stop, mute, etc.)
 * - Alerts: Overweight, overheating, jam warnings
 * 
 * MQTT Server: 192.168.43.230 (PC IP when connected to zys WiFi)
 */

#ifndef MQTT_UTIL_H
#define MQTT_UTIL_H

/**
 * @brief Connect to MQTT broker and start communication
 */
int mqtt_connect(void);

/**
 * @brief Publish sensor data to MQTT broker
 */
void mqtt_publish_status(void);

/**
 * @brief Publish alert to MQTT broker
 * @param alertType Type of alert (overweight, overheat, jam)
 */
void mqtt_publish_alert(const char *alertType);

/**
 * @brief Start MQTT communication task
 */
void mqtt_start_task(void);

#endif /* MQTT_UTIL_H */
