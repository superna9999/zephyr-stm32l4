/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 *
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

#ifndef _STM32L4X6_PINMUX_H_
#define _STM32L4X6_PINMUX_H_

/**
 * @file Header for STM32L4X6 pin multiplexing helper
 */

#define STM32L4X6_PINMUX_FUNC_PA4_SPI1_NSS STM32_PINMUX_FUNC_ALT_5
#define STM32L4X6_PINMUX_FUNC_PA5_SPI1_SCK STM32_PINMUX_FUNC_ALT_5
#define STM32L4X6_PINMUX_FUNC_PA6_SPI1_MISO STM32_PINMUX_FUNC_ALT_5
#define STM32L4X6_PINMUX_FUNC_PA7_SPI1_MOSI STM32_PINMUX_FUNC_ALT_5

#define STM32L4X6_PINMUX_FUNC_PA9_USART1_TX STM32_PINMUX_FUNC_ALT_7
#define STM32L4X6_PINMUX_FUNC_PA10_USART1_RX STM32_PINMUX_FUNC_ALT_7

#define STM32L4X6_PINMUX_FUNC_PA2_USART2_TX STM32_PINMUX_FUNC_ALT_7
#define STM32L4X6_PINMUX_FUNC_PA3_USART2_RX STM32_PINMUX_FUNC_ALT_7

#define STM32L4X6_PINMUX_FUNC_PB10_USART3_TX STM32_PINMUX_FUNC_ALT_7
#define STM32L4X6_PINMUX_FUNC_PB11_USART3_RX STM32_PINMUX_FUNC_ALT_7

#define STM32L4X6_PINMUX_FUNC_PB3_SPI3_SCK STM32_PINMUX_FUNC_ALT_6
#define STM32L4X6_PINMUX_FUNC_PB4_SPI3_MISO STM32_PINMUX_FUNC_ALT_6
#define STM32L4X6_PINMUX_FUNC_PB5_SPI3_MOSI STM32_PINMUX_FUNC_ALT_6

#define STM32L4X6_PINMUX_FUNC_PB6_I2C1_SCL STM32_PINMUX_FUNC_ALT_4
#define STM32L4X6_PINMUX_FUNC_PB7_I2C1_SDA STM32_PINMUX_FUNC_ALT_4


#endif /* _STM32L4X6_PINMUX_H_ */
