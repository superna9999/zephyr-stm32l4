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
 * @brief Driver for UART port on STM32Lx family processor.
 *
 * Chapter 36: Universal synchronous asynchronous receiver
 *             transmitter (USART)
 */

#ifndef _STM32LX_UART_H_
#define _STM32LX_UART_H_

/* 36.8.1 Control register 1 (USART_CR1) */
union __cr1 {
	uint32_t val;
	struct {
		uint32_t ue :1 __packed;
		uint32_t uesm :1 __packed;
		uint32_t re :1 __packed;
		uint32_t te :1 __packed;
		uint32_t idleie :1 __packed;
		uint32_t rxneie :1 __packed;
		uint32_t tcie :1 __packed;
		uint32_t txeie :1 __packed;
		uint32_t peie :1 __packed;
		uint32_t ps :1 __packed;
		uint32_t pce :1 __packed;
		uint32_t wake :1 __packed;
		uint32_t m0 :1 __packed;
		uint32_t mme :1 __packed;
		uint32_t cmie :1 __packed;
		uint32_t over8 :1 __packed;
		uint32_t dedt :5 __packed;
		uint32_t deat :5 __packed;
		uint32_t rtoie :1 __packed;
		uint32_t eobie :1 __packed;
		uint32_t m1 :1 __packed;
		uint32_t rsvd__29_31 :3 __packed;
	} bit;
};

/* 36.8.2 Control register 2 (USART_CR2) */
union __cr2 {
	uint32_t val;
	struct {
		uint32_t rsvd__0_4 :4 __packed;
		uint32_t addm7 :1 __packed;
		uint32_t lbdl :1 __packed;
		uint32_t lbdie :1 __packed;
		uint32_t rsvd__7 :1 __packed;
		uint32_t lbcl :1 __packed;
		uint32_t cpha :1 __packed;
		uint32_t cpol :1 __packed;
		uint32_t clken :1 __packed;
		uint32_t stop :2 __packed;
		uint32_t linen :1 __packed;
		uint32_t swap :1 __packed;
		uint32_t rxinv :1 __packed;
		uint32_t txinv :1 __packed;
		uint32_t datainv :1 __packed;
		uint32_t msbfirst :1 __packed;
		uint32_t abren :1 __packed;
		uint32_t abrmod :2 __packed;
		uint32_t rtoen :1 __packed;
		uint32_t add :8 __packed;
	} bit;
};

/* 27.6.6 Control register 3 (USART_CR3) */
union __cr3 {
	uint32_t val;
	struct {
		uint32_t eie :1 __packed;
		uint32_t iren :1 __packed;
		uint32_t irlp :1 __packed;
		uint32_t hdsel :1 __packed;
		uint32_t nack :1 __packed;
		uint32_t scen :1 __packed;
		uint32_t dmar :1 __packed;
		uint32_t dmat :1 __packed;
		uint32_t rtse :1 __packed;
		uint32_t ctse :1 __packed;
		uint32_t ctsie :1 __packed;
		uint32_t onebit :1 __packed;
		uint32_t ovrdis :1 __packed;
		uint32_t ddre :1 __packed;
		uint32_t dem :1 __packed;
		uint32_t dep :1 __packed;
		uint32_t rsvd__16 :1 __packed;
		uint32_t scarcnt :3 __packed;
		uint32_t wus :2 __packed;
		uint32_t wufie :1 __packed;
		uint32_t ucesm :1 __packed;
		uint32_t rsvd__24_31 :8 __packed;
	} bit;
};

/* 36.8.5 Guard time and prescaler register (USART_GTPR) */
union __gtpr {
	uint32_t val;
	struct {
		uint32_t psc :8 __packed;
		uint32_t gt :8 __packed;
		uint32_t rsvd__16_31 :16 __packed;
	} bit;
};

union __rtor {
	uint32_t val;
	struct {
		uint32_t rto :24 __packed;
		uint32_t blen :8 __packed;
	} bit;
};

union __rqr {
	uint32_t val;
	struct {
		uint32_t abrrq :1 __packed;
		uint32_t sbkrq :1 __packed;
		uint32_t mmrq :1 __packed;
		uint32_t rxfrq :1 __packed;
		uint32_t txfrq :1 __packed;
		uint32_t rsvd__5_31 :27 __packed;
	} bit;
};

union __isr {
	uint32_t val;
	struct {
		uint32_t pe :1 __packed;
		uint32_t fe :1 __packed;
		uint32_t nf :1 __packed;
		uint32_t ore :1 __packed;
		uint32_t idle :1 __packed;
		uint32_t rxne :1 __packed;
		uint32_t tc :1 __packed;
		uint32_t txe :1 __packed;
		uint32_t lbdf :1 __packed;
		uint32_t ctsif :1 __packed;
		uint32_t cts :1 __packed;
		uint32_t rtof :1 __packed;
		uint32_t eobf :1 __packed;
		uint32_t rsvd__13 :1 __packed;
		uint32_t abre :1 __packed;
		uint32_t abrf :1 __packed;
		uint32_t busy :1 __packed;
		uint32_t cmf :1 __packed;
		uint32_t sbkf :1 __packed;
		uint32_t rwu :1 __packed;
		uint32_t wuf :1 __packed;
		uint32_t teack :1 __packed;
		uint32_t reack :1 __packed;
		uint32_t rsvd__23_31 :9 __packed;
	} bit;
};

union __icr {
	uint32_t val;
	struct {
		uint32_t pe :1 __packed;
		uint32_t fe :1 __packed;
		uint32_t nf :1 __packed;
		uint32_t ore :1 __packed;
		uint32_t idle :1 __packed;
		uint32_t rsvd__5 :1 __packed;
		uint32_t tc :1 __packed;
		uint32_t rsvd__7 :1 __packed;
		uint32_t lbdf :1 __packed;
		uint32_t ctsif :1 __packed;
		uint32_t rsvd__10 :1 __packed;
		uint32_t rtof :1 __packed;
		uint32_t eobf :1 __packed;
		uint32_t rsvd__13_16 :4 __packed;
		uint32_t cmf :1 __packed;
		uint32_t rsvd__18_19 :2 __packed;
		uint32_t wuf :1 __packed;
		uint32_t rsvd__21_31 :9 __packed;
	} bit;
};

union __brr {
	uint32_t val;
	struct {
		uint32_t br:16 __packed;
		uint32_t rsvd__17_31 :16 __packed;
	} bit;
};

union __dr {
	uint32_t val;
	struct {
		uint32_t dr:8 __packed;
		uint32_t rsvd__9_31 :24 __packed;
	} bit;
};

/* 27.6.8 USART register map */
struct uart_stm32lx {
	union __cr1 cr1;
	union __cr2 cr2;
	union __cr3 cr3;
	union __brr brr;
	union __gtpr gtpr;
	union __rtor rtor;
	union __rqr rqr;
	union __isr isr;
	union __icr icr;
	union __dr rdr;
	union __dr tdr;
};

/* device config */
struct uart_stm32lx_config {
	struct uart_device_config uconf;
	/* clock subsystem driving this peripheral */
	clock_control_subsys_t clock_subsys;
};

/* driver data */
struct uart_stm32lx_data {
	/* current baud rate */
	uint32_t baud_rate;
	/* clock device */
	struct device *clock;
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_callback_t user_cb;
#endif
};

#endif	/* _STM32LX_UART_H_ */
