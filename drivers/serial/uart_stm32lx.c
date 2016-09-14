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
 * @brief Driver for UART port on STM32F10x family processor.
 *
 * Based on reference manual:
 *   STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx
 *   advanced ARM ® -based 32-bit MCUs
 *
 * Chapter 27: Universal synchronous asynchronous receiver
 *             transmitter (USART)
 */

#include <nanokernel.h>
#include <arch/cpu.h>
#include <misc/__assert.h>
#include <board.h>
#include <init.h>
#include <uart.h>
#include <clock_control.h>

#include <sections.h>
#include <clock_control/stm32_clock_control.h>
#include "uart_stm32lx.h"

/* convenience defines */
#define DEV_CFG(dev)							\
	((struct uart_stm32lx_config * const)(dev)->config->config_info)
#define DEV_DATA(dev)							\
	((struct uart_stm32lx_data * const)(dev)->driver_data)
#define UART_STRUCT(dev)					\
	((volatile struct uart_stm32lx *)(DEV_CFG(dev))->uconf.base)

/**
 * @brief set baud rate
 *
 */
static void set_baud_rate(struct device *dev, uint32_t rate)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);
	struct uart_stm32lx_data *data = DEV_DATA(dev);
	struct uart_stm32lx_config *cfg = DEV_CFG(dev);
	uint32_t div;
	uint32_t clock;

	/* Baud rate is controlled through BRR register. The values
	 * written into the register depend on the clock driving the
	 * peripheral. Ask clock_control for the current clock rate of
	 * our peripheral.
	 */
	clock_control_get_rate(data->clock, cfg->clock_subsys, &clock);

	/* baud rate calculation:
	 *
	 *     baud rate = f_clk / (16 * usartdiv)
	 *
	 */

	div = clock / rate;

	uart->brr.bit.br = clock / rate;
}

static int uart_stm32lx_poll_in(struct device *dev, unsigned char *c)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	/* check if RXNE is set */
	if (!uart->isr.bit.rxne) {
		return -1;
	}

	/* read character */
	*c = (unsigned char)uart->rdr.bit.dr;

	return 0;
}

static unsigned char uart_stm32lx_poll_out(struct device *dev,
					unsigned char c)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	/* wait for TXE to be set */
	while (!uart->isr.bit.txe) {
	}

	uart->tdr.bit.dr = c;
	return c;
}

static inline void __uart_stm32lx_get_clock(struct device *dev)
{
	struct uart_stm32lx_data *ddata = dev->driver_data;
	struct device *clk =
		device_get_binding(STM32_CLOCK_CONTROL_NAME);

	__ASSERT_NO_MSG(clk);

	ddata->clock = clk;
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN

static int uart_stm32lx_fifo_fill(struct device *dev, const uint8_t *tx_data,
				  int size)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);
	size_t num_tx = 0;

	/* FIXME: DMA maybe? */
	while ((size - num_tx > 0) && (uart->isr.bit.txe)) {
		uart->tdr.bit.dr = tx_data[num_tx++];
	}

	return num_tx;
}

static int uart_stm32lx_fifo_read(struct device *dev, uint8_t *rx_data,
				  const int size)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);
	size_t num_rx = 0;

	while ((size - num_rx > 0) && (uart->isr.bit.rxne)) {
		rx_data[num_rx++] = (uint8_t) uart->rdr.bit.dr;
	}
	return num_rx;
}

static void uart_stm32lx_irq_tx_enable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr1.bit.txeie = 1;
}

static void uart_stm32lx_irq_tx_disable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr1.bit.txeie = 0;
}

static int uart_stm32lx_irq_tx_ready(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	return uart->isr.bit.txe;
}

static int uart_stm32lx_irq_tx_empty(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	return uart->isr.bit.txe;
}

static void uart_stm32lx_irq_rx_enable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr1.bit.rxneie = 1;
}

static void uart_stm32lx_irq_rx_disable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr1.bit.rxneie = 0;
}

static int uart_stm32lx_irq_rx_ready(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	return uart->isr.bit.rxne;
}

static void uart_stm32lx_irq_err_enable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr3.bit.eie = 1;
}

static void uart_stm32lx_irq_err_disable(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	uart->cr3.bit.eie = 0;
}

static int uart_stm32lx_irq_is_pending(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);

	return uart->isr.bit.rxne || uart->isr.bit.txe;
}

static int uart_stm32lx_irq_update(struct device *dev)
{
	return 1;
}

static void uart_stm32lx_irq_callback_set(struct device *dev,
				       uart_irq_callback_t cb)
{
	struct uart_stm32lx_data *data = DEV_DATA(dev);

	data->user_cb = cb;
}

static void uart_stm32lx_isr(void *arg)
{
	struct device *dev = arg;
	struct uart_stm32lx_data *data = DEV_DATA(dev);

	if (data->user_cb) {
		data->user_cb(dev);
	}
}

#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_driver_api uart_stm32lx_driver_api = {
	.poll_in = uart_stm32lx_poll_in,
	.poll_out = uart_stm32lx_poll_out,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.fifo_fill = uart_stm32lx_fifo_fill,
	.fifo_read = uart_stm32lx_fifo_read,
	.irq_tx_enable = uart_stm32lx_irq_tx_enable,
	.irq_tx_disable = uart_stm32lx_irq_tx_disable,
	.irq_tx_ready = uart_stm32lx_irq_tx_ready,
	.irq_tx_empty = uart_stm32lx_irq_tx_empty,
	.irq_rx_enable = uart_stm32lx_irq_rx_enable,
	.irq_rx_disable = uart_stm32lx_irq_rx_disable,
	.irq_rx_ready = uart_stm32lx_irq_rx_ready,
	.irq_err_enable = uart_stm32lx_irq_err_enable,
	.irq_err_disable = uart_stm32lx_irq_err_disable,
	.irq_is_pending = uart_stm32lx_irq_is_pending,
	.irq_update = uart_stm32lx_irq_update,
	.irq_callback_set = uart_stm32lx_irq_callback_set,
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */
};

/**
 * @brief Initialize UART channel
 *
 * This routine is called to reset the chip in a quiescent state.
 * It is assumed that this function is called only once per UART.
 *
 * @param dev UART device struct
 *
 * @return 0
 */
static int uart_stm32lx_init(struct device *dev)
{
	volatile struct uart_stm32lx *uart = UART_STRUCT(dev);
	struct uart_stm32lx_data *data = DEV_DATA(dev);
	struct uart_stm32lx_config *cfg = DEV_CFG(dev);

	__uart_stm32lx_get_clock(dev);

	/* enable clock */
	clock_control_on(data->clock, cfg->clock_subsys);

	/* FIXME: hardcoded, clear stop bits */
	uart->cr2.bit.stop = 0;

	uart->cr1.val = 0;
	/* FIXME: hardcoded, 8n1 */
	uart->cr1.bit.m0 = 0;
	uart->cr1.bit.m1 = 0;
	uart->cr1.bit.pce = 0;

	/* FIXME: hardcoded, disable hardware flow control */
	uart->cr3.bit.ctse = 0;
	uart->cr3.bit.rtse = 0;

	set_baud_rate(dev, data->baud_rate);

	/* enable TX/RX */
	uart->cr1.bit.te = 1;
	uart->cr1.bit.re = 1;

	/* enable */
	uart->cr1.bit.ue = 1;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	cfg->uconf.irq_config_func(dev);
#endif
	return 0;
}

#ifdef CONFIG_UART_STM32LX_PORT_0

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_0(struct device *dev);
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_stm32lx_config uart_stm32lx_dev_cfg_0 = {
	.uconf = {
		.base = (uint8_t *)USART1_ADDR,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
		.irq_config_func = uart_stm32lx_irq_config_func_0,
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */
	},
#if defined(CONFIG_SOC_SERIES_STM32L4X)
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_USART1),
#endif	/* CONFIG_SOC_SERIES_STM32F1X */
};

static struct uart_stm32lx_data uart_stm32lx_dev_data_0 = {
	.baud_rate = CONFIG_UART_STM32LX_PORT_0_BAUD_RATE,
};

DEVICE_AND_API_INIT(uart_stm32lx_0, CONFIG_UART_STM32LX_PORT_0_NAME,
		    &uart_stm32lx_init,
		    &uart_stm32lx_dev_data_0, &uart_stm32lx_dev_cfg_0,
		    PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &uart_stm32lx_driver_api);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_0(struct device *dev)
{
#if defined(CONFIG_SOC_SERIES_STM32L4X)
#define PORT_0_IRQ STM32L4_IRQ_USART1
#endif
	IRQ_CONNECT(PORT_0_IRQ,
		CONFIG_UART_STM32LX_PORT_0_IRQ_PRI,
		uart_stm32lx_isr, DEVICE_GET(uart_stm32lx_0),
		0);
	irq_enable(PORT_0_IRQ);
}
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

#endif	/* CONFIG_UART_STM32LX_PORT_0 */

#ifdef CONFIG_UART_STM32LX_PORT_1

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_1(struct device *dev);
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_stm32lx_config uart_stm32lx_dev_cfg_1 = {
	.uconf = {
		.base = (uint8_t *)USART2_ADDR,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
		.irq_config_func = uart_stm32lx_irq_config_func_1,
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */
	},
#if defined(CONFIG_SOC_SERIES_STM32L4X)
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_USART2),
#endif	/* CONFIG_SOC_SERIES_STM32F1X */
};

static struct uart_stm32lx_data uart_stm32lx_dev_data_1 = {
	.baud_rate = CONFIG_UART_STM32LX_PORT_1_BAUD_RATE,
};

DEVICE_AND_API_INIT(uart_stm32lx_1, CONFIG_UART_STM32LX_PORT_1_NAME,
		    &uart_stm32lx_init,
		    &uart_stm32lx_dev_data_1, &uart_stm32lx_dev_cfg_1,
		    PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &uart_stm32lx_driver_api);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_1(struct device *dev)
{
#if defined(CONFIG_SOC_SERIES_STM32L4X)
#define PORT_2_IRQ STM32L4_IRQ_USART2
#endif
	IRQ_CONNECT(PORT_1_IRQ,
		CONFIG_UART_STM32LX_PORT_1_IRQ_PRI,
		uart_stm32lx_isr, DEVICE_GET(uart_stm32lx_1),
		0);
	irq_enable(PORT_1_IRQ);
}
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

#endif	/* CONFIG_UART_STM32LX_PORT_1 */

#ifdef CONFIG_UART_STM32LX_PORT_2

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_2(struct device *dev);
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_stm32lx_config uart_stm32lx_dev_cfg_2 = {
	.uconf = {
		.base = (uint8_t *)USART3_ADDR,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
		.irq_config_func = uart_stm32lx_irq_config_func_2,
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */
	},
#if defined(CONFIG_SOC_SERIES_STM32L4X)
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_USART3),
#endif	/* CONFIG_SOC_SERIES_STM32F1X */
};

static struct uart_stm32lx_data uart_stm32lx_dev_data_2 = {
	.baud_rate = CONFIG_UART_STM32LX_PORT_2_BAUD_RATE,
};

DEVICE_AND_API_INIT(uart_stm32lx_2, CONFIG_UART_STM32LX_PORT_2_NAME,
		    &uart_stm32lx_init,
		    &uart_stm32lx_dev_data_2, &uart_stm32lx_dev_cfg_2,
		    PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &uart_stm32lx_driver_api);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_stm32lx_irq_config_func_2(struct device *dev)
{
#if defined(CONFIG_SOC_SERIES_STM32L4X)
#define PORT_3_IRQ STM32L4_IRQ_USART3
#endif
	IRQ_CONNECT(PORT_2_IRQ,
		CONFIG_UART_STM32LX_PORT_2_IRQ_PRI,
		uart_stm32lx_isr, DEVICE_GET(uart_stm32lx_2),
		0);
	irq_enable(PORT_2_IRQ);
}
#endif	/* CONFIG_UART_INTERRUPT_DRIVEN */

#endif	/* CONFIG_UART_STM32LX_PORT_2 */
