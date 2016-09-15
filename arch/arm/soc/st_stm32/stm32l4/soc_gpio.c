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
 * @brief
 *
 * Based on reference manual:
 *   STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx
 *   advanced ARM ® -based 32-bit MCUs
 *
 * Chapter 9: General-purpose and alternate-function I/Os
 *            (GPIOs and AFIOs)
 */

#include <errno.h>

#include <device.h>
#include "soc.h"
#include "soc_registers.h"
#include <gpio.h>
#include <gpio/gpio_stm32.h>

/**
 * @brief map pin function to MODE register value
 */
static uint32_t __func_to_mode(int conf, unsigned int afnum)
{
	/* If an alternate function is specified */
	if (afnum)
		return 0x2;

	switch (conf) {
	case STM32L4X6_PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
	case STM32L4X6_PIN_CONFIG_BIAS_PULL_UP:
	case STM32L4X6_PIN_CONFIG_BIAS_PULL_DOWN:
		return 0;
	case STM32L4X6_PIN_CONFIG_ANALOG:
		return 0x3;
	default:
		return 0x1;
	}
	return 0;
}

static uint32_t __func_to_otype(int conf)
{
	switch (conf) {
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN:
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_UP:
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_DOWN:
		return 0x1;
	default:
		return 0x0;
	}
	return 0;
}

static uint32_t __func_to_pupd(int conf)
{
	switch (conf) {
	case STM32L4X6_PIN_CONFIG_ANALOG:
	case STM32L4X6_PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
	case STM32L4X6_PIN_CONFIG_PUSH_PULL:
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN:
		return 0x0;
	case STM32L4X6_PIN_CONFIG_BIAS_PULL_UP:
	case STM32L4X6_PIN_CONFIG_PUSH_PULL_PULL_UP:
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_UP:
		return 0x1;
	case STM32L4X6_PIN_CONFIG_BIAS_PULL_DOWN:
	case STM32L4X6_PIN_CONFIG_PUSH_PULL_PULL_DOWN:
	case STM32L4X6_PIN_CONFIG_OPEN_DRAIN_PULL_DOWN:
		return 0x2;
	}
	return 0;
}

int stm32_gpio_flags_to_conf(int flags, int *pincfg)
{
	int direction = flags & GPIO_DIR_MASK;

	if (!pincfg) {
		return -EINVAL;
	}

	if (direction == GPIO_DIR_OUT) {
		*pincfg = STM32L4X6_PIN_CONFIG_PUSH_PULL;
	} else if (direction == GPIO_DIR_IN) {
		int pud = flags & GPIO_PUD_MASK;

		/* pull-{up,down} maybe? */
		if (pud == GPIO_PUD_PULL_UP) {
			*pincfg = STM32L4X6_PIN_CONFIG_BIAS_PULL_UP;
		} else if (pud == GPIO_PUD_PULL_DOWN) {
			*pincfg = STM32L4X6_PIN_CONFIG_BIAS_PULL_DOWN;
		} else {
			/* floating */
			*pincfg = STM32L4X6_PIN_CONFIG_BIAS_HIGH_IMPEDANCE;
		}
	} else {
		return -ENOTSUP;
	}

	return 0;
}

int stm32_gpio_configure(uint32_t *base_addr, int pin, int conf)
{
	volatile struct stm32l4x6_gpio *gpio =
		(struct stm32l4x6_gpio *)(base_addr);
	unsigned int afnum = STM32L4X6_PIN_AF(conf) & 0xf;
	unsigned int pinconf = STM32L4X6_PIN_CONFIG(conf);
	unsigned int mode, otype, pupd;
	unsigned int pin_shift = pin << 1;
	unsigned int afr_bank = pin / 8;
	unsigned int afr_shift = (pin % 8) << 2;
	uint32_t scratch;

	mode = __func_to_mode(pinconf, afnum);
	otype = __func_to_otype(pinconf);
	pupd = __func_to_pupd(pinconf);

	scratch = gpio->moder & ~(0x3 << pin_shift);
	gpio->moder = scratch | (mode << pin_shift);

	scratch = gpio->otyper & ~(0x1 << pin);
	gpio->otyper = scratch | (otype << pin);

	scratch = gpio->pupdr & ~(0x3 << pin_shift);
	gpio->pupdr = scratch | (pupd << pin_shift);

	scratch = gpio->afr[afr_bank] & ~(0xf << afr_shift);
	gpio->afr[afr_bank] = scratch | (afnum << afr_shift);

	return 0;
}

int stm32_gpio_set(uint32_t *base, int pin, int value)
{
	struct stm32l4x6_gpio *gpio = (struct stm32l4x6_gpio *)base;

	int pval = 1 << (pin & 0xf);

	if (value) {
		gpio->odr |= pval;
	} else {
		gpio->odr &= ~pval;
	}

	return 0;
}

int stm32_gpio_get(uint32_t *base, int pin)
{
	struct stm32l4x6_gpio *gpio = (struct stm32l4x6_gpio *)base;

	return (gpio->idr >> pin) & 0x1;
}

int stm32_gpio_enable_int(int port, int pin)
{
	/* TODO */

	return 0;
}
