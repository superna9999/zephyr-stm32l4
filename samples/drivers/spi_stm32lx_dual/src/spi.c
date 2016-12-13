/*
 * Copyright (c) 2016 BayLibre, SAS.
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

#include <zephyr.h>

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#define PRINT printf
#else
#include <misc/printk.h>
#define PRINT printk
#endif

#include <string.h>
#include <spi.h>

#include <drivers/spi/spi_stm32lx.h>
#define SPI_SLAVE_MAX_CLK_FREQ_HZ 10000000
#define SPI_MASTER_MAX_CLK_FREQ_HZ 1000000
#define SPI_MASTER_DRV_NAME "SPI_1"
#define SPI_SLAVE_DRV_NAME "SPI_3"

static void print_buf_hex(unsigned char *b, uint32_t len)
{
	for (; len > 0; len--) {
		PRINT("0x%x ", *(b++));
	}

	PRINT("\n");
}

struct spi_config spi_conf_slave = {
	.config = SPI_MODE_CPOL | SPI_MODE_CPHA | (8 << 4) | STM32LX_SPI_SLAVE_MODE | STM32LX_SPI_SLAVE_NSS_IGNORE,
	.max_sys_freq = SPI_SLAVE_MAX_CLK_FREQ_HZ,
};

struct spi_config spi_conf_master = {
	.config = SPI_MODE_CPOL | SPI_MODE_CPHA | (8 << 4) | STM32LX_SPI_MASTER_NSS_IGNORE,
	.max_sys_freq = SPI_MASTER_MAX_CLK_FREQ_HZ,
};

static void _spi_show(struct spi_config *spi_conf)
{
	PRINT("SPI Configuration:\n");
	PRINT("\tbits per word: %u\n", SPI_WORD_SIZE_GET(spi_conf->config));
	PRINT("\tMode: %u\n", SPI_MODE(spi_conf->config));
	PRINT("\tMax speed : %dHz\n", spi_conf->max_sys_freq);
}

void task_slave(void *unused0, void *unused1, void *unused2)
{
	unsigned char wbuf[6];
	unsigned char rbuf[6];
	struct device *spi;

	spi = device_get_binding(SPI_SLAVE_DRV_NAME);

	PRINT("SPI Slave Running...\n");

	spi_configure(spi, &spi_conf_slave);

	_spi_show(&spi_conf_slave);

	while (1) {
		strcpy(wbuf, "123456");
		spi_transceive(spi, wbuf, 6, rbuf, 6);

		PRINT("SPI Slave Received: %s\n", rbuf);
		print_buf_hex(rbuf, 6);
	}
}

void task_master(void *unused0, void *unused1, void *unused2)
{

	unsigned char wbuf[6];
	unsigned char rbuf[6];
	struct device *spi;

	spi = device_get_binding(SPI_MASTER_DRV_NAME);

	PRINT("SPI Master Running...\n");

	spi_configure(spi, &spi_conf_master);

	_spi_show(&spi_conf_master);

	while (1) {
		strcpy(wbuf, "ABCDEF");
		spi_transceive(spi, wbuf, 6, rbuf, 6);

		PRINT("SPI Master Received: %s\n", rbuf);
		print_buf_hex(rbuf, 6);

		k_sleep(SECONDS(1));
	}
}

#define STACK_SIZE 1024
static char __stack stacks[2][STACK_SIZE];

void main(void)
{
	k_thread_spawn(&stacks[0][0], STACK_SIZE,
			task_slave, NULL, NULL, NULL, 7, 0, 0);
	k_thread_spawn(&stacks[1][0], STACK_SIZE,
			task_master, NULL, NULL, NULL, 7, 0, 0);
}
