# 工程复制

只需保留 `.ewd`, `.ewp`, `.eww`

# 串口配置

注意：

- 默认波特率 38400
- 关闭硬件流控：`MT_UART_DEFAULT_OVERFLOW=FALSE`

# 装配说明

## on/off light (router)

- output: P1_0 (`ACTIVE_LOW`)
- online flag: P1_1 (`ACTIVE_LOW`)

## gray light (router)

- output: P1_0 (`ACTIVE_HIGH`)
- output(minor): P1_1 (`ACTIVE_HIGH`)
- online flag: P1_1 (`ACTIVE_LOW`)

## 温度传感器

- IO: P0_7
- online flag: P1_1 (`ACTIVE_LOW`)

## 照度传感器

- SDA: P1_2
- SCL: P1_3
- ADD: LOW
- online flag: P1_1 (`ACTIVE_LOW`)