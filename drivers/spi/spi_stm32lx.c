 /*
 * Copyright (c) 2016 BayLibre, SAS
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

#include <errno.h>

#include <nanokernel.h>

#include <board.h>
#include <spi.h>
#include <clock_control.h>

#include <misc/util.h>

#include <clock_control/stm32_clock_control.h>
#include "spi_stm32lx.h"
#include <drivers/spi/spi_stm32lx.h>

#define SYS_LOG_LEVEL CONFIG_SYS_LOG_SPI_LEVEL
#include <misc/sys_log.h>

/* convenience defines */
#define DEV_CFG(dev)							\
	((struct spi_stm32lx_config * const)(dev)->config->config_info)
#define DEV_DATA(dev)							\
	((struct spi_stm32lx_data * const)(dev)->driver_data)
#define SPI_STRUCT(dev)							\
	((volatile struct spi_stm32lx *)(DEV_CFG(dev))->base)


#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_isr(void *arg)
{
	struct device * const dev = (struct device *)arg;
	volatile struct spi_stm32lx *spi = SPI_STRUCT(dev);
	struct spi_stm32lx_data *data = DEV_DATA(dev);
	volatile uint8_t *dr16 = (void *)&spi->dr;
	volatile uint8_t *dr8 = (void *)&spi->dr;

	if (spi->sr.bit.txe && data->current.tx_len) {
		if (data->current.is_ds_16) {
			uint16_t *b16 = (void *)data->current.tx_buf;
			*dr16 = *b16;
			data->current.tx_buf++;
			data->current.tx_len--;
		}
		else
			*dr8 = *data->current.tx_buf;
		data->current.tx_buf++;
		data->current.tx_len--;
	}

	if (spi->sr.bit.rxne && data->current.rx_len) {
		if (data->current.is_ds_16) {
			uint16_t *b16 = (void *)data->current.rx_buf;
			*b16 = *dr16;
			data->current.rx_buf++;
			data->current.rx_len--;
		}
		else
			*data->current.rx_buf = *dr8;
		data->current.rx_buf++;
		data->current.rx_len--;
	}

	if (!data->current.rx_len && !data->current.tx_len) {
		spi->cr2.bit.rxneie = 0;
		spi->cr2.bit.txeie = 0;

		device_sync_call_complete(&data->sync);
	}
}
#endif

#define BR_SCALER_COUNT		8
static const uint32_t baud_rate_scaler[] = {
	2, 4, 8, 16, 32, 64, 128, 256
};

/* Only support 8bit or 16bit data size */
#define DATA_SIZE_8_CFG		(3 + 4)
#define DATA_SIZE_16_CFG	(3 + 15)

static int spi_stm32lx_configure(struct device *dev, struct spi_config *config)
{
	volatile struct spi_stm32lx *spi = SPI_STRUCT(dev);
	struct spi_stm32lx_data *data = DEV_DATA(dev);
	struct spi_stm32lx_config *cfg = DEV_CFG(dev);
	uint32_t flags = config->config;
	uint32_t clock;
	unsigned ds;
	int br;

	clock_control_get_rate(data->clock, cfg->clock_subsys, &clock);

	if (SPI_WORD_SIZE_GET(flags) == 8)
		ds = DATA_SIZE_8_CFG;
	else if (SPI_WORD_SIZE_GET(flags) == 16)
		ds = DATA_SIZE_16_CFG;
	else
		return -ENOTSUP;

	for (br = 0 ; br < BR_SCALER_COUNT ; ++br) {
		uint32_t clk = clock / baud_rate_scaler[br];

		if (clk < config->max_sys_freq)
			break;
	}

	if (br >= BR_SCALER_COUNT)
		return -ENOTSUP;

	/* Disable Peripheral */
	spi->cr1.bit.spe = 0;

	/* Setup baud rate prescaler */
	spi->cr1.bit.br = br;

	/* Setup polarity flags */
	if (flags & SPI_MODE_CPOL)
		spi->cr1.bit.cpol = 1;
	else
		spi->cr1.bit.cpol = 0;

	if (flags & SPI_MODE_CPHA)
		spi->cr1.bit.cpha = 1;
	else
		spi->cr1.bit.cpha = 0;

	/* Full Duplex 2-lines */
	spi->cr1.bit.rxonly = 0;
	spi->cr1.bit.bidimode = 0;

	/* Setup transfer bit mode */
	if (flags & SPI_TRANSFER_MASK)
		spi->cr1.bit.lsbfirst = 1;
	else
		spi->cr1.bit.lsbfirst = 0;

	/* Disable CRC Feature */
	spi->cr1.bit.crcen = 0;

	/* Slave Support */
	if (flags & STM32LX_SPI_SLAVE_MODE) {
		spi->cr1.bit.mstr = 0;

		/* NSS Management */
		if (flags & STM32LX_SPI_SLAVE_NSS_IGNORE) {
			spi->cr1.bit.ssm = 1;
			spi->cr1.bit.ssi = 0;
		}
		else
			spi->cr1.bit.ssm = 0;

		data->current.is_slave = 1;
	} else {
		spi->cr1.bit.mstr = 1;
		spi->cr1.bit.ssm = 0;

		/* NSS Management */
		if (flags & STM32LX_SPI_MASTER_NSS_IGNORE) {
			spi->cr2.bit.ssoe = 0;
		} else {
			spi->cr2.bit.ssoe = 1;
			spi->cr2.bit.nssp = 1;
		}

		data->current.is_slave = 0;
	}

	/* Setup Data size */
	spi->cr2.bit.ds = ds;

	/* Motorola Format */
	spi->cr2.bit.frf = 0;

	if (ds == DATA_SIZE_16_CFG)
		spi->cr2.bit.frxth = 0;
	else
		spi->cr2.bit.frxth = 1;

	data->current.rx_len = 0;
	data->current.rx_buf = NULL;
	data->current.tx_len = 0;
	data->current.tx_buf = NULL;

	if (ds == DATA_SIZE_16_CFG)
		data->current.is_ds_16 = 1;
	else
		data->current.is_ds_16 = 0;

	return 0;
}

static int spi_stm32lx_slave_select(struct device *dev, uint32_t slave)
{
	/* NOP */

	return 0;
}

static int spi_stm32lx_transceive(struct device *dev,
				const void *tx_buf, uint32_t tx_buf_len,
				void *rx_buf, uint32_t rx_buf_len)
{
	volatile struct spi_stm32lx *spi = SPI_STRUCT(dev);
	struct spi_stm32lx_data *data = DEV_DATA(dev);
#ifndef CONFIG_SPI_STM32LX_INTERRUPT
	volatile uint8_t *dr16 = (void *)&spi->dr;
#endif
	volatile uint8_t *dr8 = (void *)&spi->dr;

	__ASSERT(!((tx_buf_len && (tx_buf == NULL)) ||
				(rx_buf_len && (rx_buf == NULL))),
			"spi_stm32lx_transceive: ERROR - NULL buffer");

	__ASSERT(!(data->current.is_ds_16 &&
				((tx_buf_len % 2) || (rx_buf_len % 2))),
			"spi_stm32lx_transceive: invalid size for data size cfg");

	data->current.rx_len = rx_buf_len;
	data->current.rx_buf = rx_buf;
	data->current.tx_len = tx_buf_len;
	data->current.tx_buf = tx_buf;

	/* Enable Peripheral */
	spi->cr1.bit.spe = 1;

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
	if (rx_buf_len)
		spi->cr2.bit.rxneie = 1;

	if (tx_buf_len)
		spi->cr2.bit.txeie = 1;

	device_sync_call_wait(&data->sync);
#else
	do {
		if (spi->sr.bit.txe && data->current.tx_len) {
			if (data->current.is_ds_16) {
				uint16_t *b16 = (void *)data->current.tx_buf;
				*dr16 = *b16;
				data->current.tx_buf++;
				data->current.tx_len--;
			}
			else {
				*dr8 = *data->current.tx_buf;
			}
			data->current.tx_buf++;
			data->current.tx_len--;
		}

		if (spi->sr.bit.rxne && data->current.rx_len) {
			if (data->current.is_ds_16) {
				uint16_t *b16 = (void *)data->current.rx_buf;
				*b16 = *dr16;
				data->current.rx_buf++;
				data->current.rx_len--;
			}
			else
				*data->current.rx_buf = *dr8;
			data->current.rx_buf++;
			data->current.rx_len--;
		}

	} while (data->current.tx_len || data->current.rx_len);
#endif

	/* Empty the RX FIFO */
	while (spi->sr.bit.frlvl) {
		volatile uint32_t tmpreg;
		tmpreg = *dr8;
		(void)tmpreg;
	}

	if (!data->current.is_slave) {
		while(spi->sr.bit.bsy);

		/* Disable Peripheral */
		spi->cr1.bit.spe = 0;
	}

	return 0;
}

static struct spi_driver_api api_funcs = {
	.configure = spi_stm32lx_configure,
	.slave_select = spi_stm32lx_slave_select,
	.transceive = spi_stm32lx_transceive,
};

static inline void __spi_stm32lx_get_clock(struct device *dev)
{
	struct spi_stm32lx_data *data = DEV_DATA(dev);
	struct device *clk =
		device_get_binding(STM32_CLOCK_CONTROL_NAME);

	__ASSERT_NO_MSG(clk);

	data->clock = clk;
}

static int spi_stm32lx_init(struct device *dev)
{
	volatile struct spi_stm32lx *spi = SPI_STRUCT(dev);
	struct spi_stm32lx_data *data = DEV_DATA(dev);
	struct spi_stm32lx_config *cfg = DEV_CFG(dev);

	device_sync_call_init(&data->sync);

	__spi_stm32lx_get_clock(dev);

	/* enable clock */
	clock_control_on(data->clock, cfg->clock_subsys);

	/* Reset config */
	spi->cr1.val = 0;
	spi->cr2.val = 0;
	spi->sr.val = 0;

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
	cfg->irq_config_func(dev);
#endif

	return 0;
}

#ifdef CONFIG_SPI_0

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_0(struct device *port);
#endif

static struct spi_stm32lx_config spi_stm32lx_cfg_0 = {
	.base = (uint8_t *)SPI1_ADDR,
#ifdef CONFIG_SOC_SERIES_STM32L4X
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_SPI1),
#endif
#ifdef CONFIG_SPI_STM32LX_INTERRUPT
	.irq_config_func = spi_stm32lx_irq_config_func_0,
#endif
};

static struct spi_stm32lx_data spi_stm32lx_dev_data_0 = {
};

DEVICE_AND_API_INIT(spi_stm32lx_0, CONFIG_SPI_0_NAME, &spi_stm32lx_init,
		    &spi_stm32lx_dev_data_0, &spi_stm32lx_cfg_0,
		    SECONDARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_0(struct device *dev)
{
#ifdef CONFIG_SOC_SERIES_STM32L4X
#define PORT_0_IRQ STM32L4_IRQ_SPI1
#endif
	IRQ_CONNECT(PORT_0_IRQ, CONFIG_SPI_0_IRQ_PRI,
		spi_stm32lx_isr, DEVICE_GET(spi_stm32lx_0), 0);
	irq_enable(PORT_0_IRQ);
}
#endif

#endif /* CONFIG_SPI_0 */

#ifdef CONFIG_SPI_1

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_1(struct device *port);
#endif

static struct spi_stm32lx_config spi_stm32lx_cfg_1 = {
	.base = (uint8_t *)SPI2_ADDR,
#ifdef CONFIG_SOC_SERIES_STM32L4X
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_SPI2),
#endif
#ifdef CONFIG_SPI_STM32LX_INTERRUPT
	.irq_config_func = spi_stm32lx_irq_config_func_1,
#endif
};

static struct spi_stm32lx_data spi_stm32lx_dev_data_1 = {
};

DEVICE_AND_API_INIT(spi_stm32lx_1, CONFIG_SPI_1_NAME, &spi_stm32lx_init,
		    &spi_stm32lx_dev_data_1, &spi_stm32lx_cfg_1,
		    SECONDARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_1(struct device *dev)
{
#ifdef CONFIG_SOC_SERIES_STM32L4X
#define PORT_1_IRQ STM32L4_IRQ_SPI2
#endif
	IRQ_CONNECT(PORT_1_IRQ, CONFIG_SPI_1_IRQ_PRI,
		spi_stm32lx_isr, DEVICE_GET(spi_stm32lx_1), 0);
	irq_enable(PORT_1_IRQ);
}
#endif

#endif /* CONFIG_SPI_1 */

#ifdef CONFIG_SPI_2

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_2(struct device *port);
#endif

static struct spi_stm32lx_config spi_stm32lx_cfg_2 = {
	.base = (uint8_t *)SPI3_ADDR,
#ifdef CONFIG_SOC_SERIES_STM32L4X
	.clock_subsys = UINT_TO_POINTER(STM32L4X6_CLOCK_SUBSYS_SPI3),
#endif
#ifdef CONFIG_SPI_STM32LX_INTERRUPT
	.irq_config_func = spi_stm32lx_irq_config_func_2,
#endif
};

static struct spi_stm32lx_data spi_stm32lx_dev_data_2 = {
};

DEVICE_AND_API_INIT(spi_stm32lx_2, CONFIG_SPI_2_NAME, &spi_stm32lx_init,
		    &spi_stm32lx_dev_data_2, &spi_stm32lx_cfg_2,
		    SECONDARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32LX_INTERRUPT
static void spi_stm32lx_irq_config_func_2(struct device *dev)
{
#ifdef CONFIG_SOC_SERIES_STM32L4X
#define PORT_2_IRQ STM32L4_IRQ_SPI3
#endif
	IRQ_CONNECT(PORT_2_IRQ, CONFIG_SPI_2_IRQ_PRI,
		spi_stm32lx_isr, DEVICE_GET(spi_stm32lx_2), 0);
	irq_enable(PORT_2_IRQ);
}
#endif

#endif /* CONFIG_SPI_2 */
