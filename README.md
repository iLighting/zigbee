# 串口配置

注意：

- 默认波特率 38400
- 关闭硬件流控：`MT_UART_DEFAULT_OVERFLOW=FALSE`

# 装配说明

## on/off light (router)

- output: P1_0 (`ACTIVE_LOW`)
- online flag: P1_1 (`ACTIVE_LOW`)

## gray light (router)

- output: P1_0 (`ACTIVE_LOW`)
- online flag: P1_1 (`ACTIVE_LOW`)

# 硬件清单

## on/off light

- [ ] 3.3v继电器 / TIP127

## gray light

?

## sensor-light

- [x] 照度模块

## sensor-temperature

- [x] 温度模块

## sensor-fire

- [ ] 火焰传感器

## sensor-occupy

- [x] 人体红外传感器
