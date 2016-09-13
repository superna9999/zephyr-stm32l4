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

/**
 * @file SoC configuration macros for the STM32F103 family processors.
 *
 * Based on reference manual:
 *   STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx
 *   advanced ARM ® -based 32-bit MCUs
 *
 * Chapter 3.3: Memory Map
 */


#ifndef _STM32F1_SOC_H_
#define _STM32F1_SOC_H_

/* peripherals start address */
#define PERIPH_BASE           0x40000000

/* use naming consistent with STML4X6 Peripherals Library */
#define APB1PERIPH_BASE        PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x20000)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x8000000)

/* UART */
#define USART1_ADDR           (APB2PERIPH_BASE + 0x3800)
#define USART2_ADDR           (APB1PERIPH_BASE + 0x4400)
#define USART3_ADDR           (APB1PERIPH_BASE + 0x4800)
#define UART4_ADDR            (APB1PERIPH_BASE + 0x4C00)
#define UART5_ADDR            (APB1PERIPH_BASE + 0x5000)
#define LPUART1_ADDR          (APB1PERIPH_BASE + 0x8000)

/* Reset and Clock Control */
#define RCC_BASE              (AHB1PERIPH_BASE + 0x1000)

#define GPIO_REG_SIZE         0x400
#define GPIOA_BASE            (AHB2PERIPH_BASE + 0x0000)
#define GPIOB_BASE            (AHB2PERIPH_BASE + 0x0400)
#define GPIOC_BASE            (AHB2PERIPH_BASE + 0x0800)
#define GPIOD_BASE            (AHB2PERIPH_BASE + 0x0C00)
#define GPIOE_BASE            (AHB2PERIPH_BASE + 0x1000)
#define GPIOF_BASE            (AHB2PERIPH_BASE + 0x1400)
#define GPIOG_BASE            (AHB2PERIPH_BASE + 0x1800)
#define GPIOH_BASE            (AHB2PERIPH_BASE + 0x1C00)
/* base address for where GPIO registers start */
#define GPIO_PORTS_BASE       (GPIOA_BASE)

/* EXTI */
#define EXTI_BASE            (APB2PERIPH_BASE + 0x0400)

/* IWDG */
#define IWDG_BASE            (APB1PERIPH_BASE + 0x3000)

/* FLASH */
#define FLASH_BASE           (AHB1PERIPH_BASE + 0x2000)

#ifndef _ASMLANGUAGE

#include <device.h>
#include <misc/util.h>
#include <drivers/rand32.h>

/* IO pin functions */
enum stm32l4x6_pin_config_mode {
	STM32L4X6_PIN_CONFIG_BIAS_HIGH_IMPEDANCE = 0,
	STM32L4X6_PIN_CONFIG_BIAS_PULL_UP,
	STM32L4X6_PIN_CONFIG_BIAS_PULL_DOWN,
	STM32L4X6_PIN_CONFIG_ANALOG,
	STM32L4X6_PIN_CONFIG_OPEN_DRAIN,
	STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_UP,
	STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_DOWN,
	STM32L4X6_PIN_CONFIG_PUSH_PULL,
	STM32L4X6_PIN_CONFIG_PUSH_PULL_PULL_UP,
	STM32L4X6_PIN_CONFIG_PUSH_PULL_PULL_DOWN,
};

#define STM32L4X6_PIN(func, conf)	(((func) & 0xFFFF) | ((conf ) << 16))

#define STM32L4X6_PIN_AF(conf)		((conf ) & 0xFFFF)
#define STM32L4X6_PIN_CONFIG(conf)	((conf ) >> 16)

#include "soc_irq.h"

#endif /* !_ASMLANGUAGE */

#endif /* _STM32F1_SOC_H_ */
