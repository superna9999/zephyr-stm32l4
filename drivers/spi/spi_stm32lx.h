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

#ifndef _STM32LX_SPI_H_
#define _STM32LX_SPI_H_

/* 38.6.1 Control register 1 (SPI_CR1) */
union __cr1 {
	uint32_t val;
	struct {
		uint32_t cpha :1 __packed;
		uint32_t cpol :1 __packed;
		uint32_t mstr :1 __packed;
		uint32_t br :3 __packed;
		uint32_t spe :1 __packed;
		uint32_t lsbfirst :1 __packed;
		uint32_t ssi :1 __packed;
		uint32_t ssm :1 __packed;
		uint32_t rxonly :1 __packed;
		uint32_t crcl :1 __packed;
		uint32_t crcnext :1 __packed;
		uint32_t crcen :1 __packed;
		uint32_t bidioe :1 __packed;
		uint32_t bidimode :1 __packed;
		uint32_t rsvd__16_31 :16 __packed;
	} bit;
};

/* 38.6.2 Control register 2 (SPI_CR2) */
union __cr2 {
	uint32_t val;
	struct {
		uint32_t rxdmaen :1 __packed;
		uint32_t txdmaen :1 __packed;
		uint32_t ssoe :1 __packed;
		uint32_t nssp :1 __packed;
		uint32_t frf :1 __packed;
		uint32_t errie :1 __packed;
		uint32_t rxneie :1 __packed;
		uint32_t txeie :1 __packed;
		uint32_t ds :4 __packed;
		uint32_t frxth :1 __packed;
		uint32_t ldma_rx :1 __packed;
		uint32_t ldma_tx :1 __packed;
		uint32_t rsvd__15_31 :17 __packed;
	} bit;
};

union __sr {
	uint32_t val;
	struct {
		uint32_t rxne :1 __packed;
		uint32_t txe :1 __packed;
		uint32_t rsvd__2_3 :2 __packed;
		uint32_t crcerr :1 __packed;
		uint32_t modf :1 __packed;
		uint32_t ovr :1 __packed;
		uint32_t bsy :1 __packed;
		uint32_t fre :1 __packed;
		uint32_t frlvl :2 __packed;
		uint32_t ftlvl :2 __packed;
		uint32_t rsvd__13_31 :19 __packed;
	} bit;
};

/* 36.6.8 SPI register map */
struct spi_stm32lx {
	union __cr1 cr1;
	union __cr2 cr2;
	union __sr sr;
	uint32_t dr;
	uint32_t crcpr;
	uint32_t rxcrcpr;
	uint32_t txcrcpr;
};

typedef void (*irq_config_func_t)(struct device *port);

/* device config */
struct spi_stm32lx_config {
	void *base;
	irq_config_func_t irq_config_func;
	/* clock subsystem driving this peripheral */
	clock_control_subsys_t clock_subsys;
};

/* driver data */
struct spi_stm32lx_data {
	/* clock device */
	struct device *clock;
	/* ISR Sync */
	device_sync_call_t sync;
	/* Current message data */
	struct {
		unsigned is_ds_16;
		unsigned rx_len;
		uint8_t *rx_buf;
		unsigned tx_len;
		const uint8_t *tx_buf;
		unsigned is_err;
	} current;
};

#endif	/* _STM32LX_SPI_H_ */
