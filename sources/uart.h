#ifndef __UART_H__
#define __UART_H__


#define MAX_BUF    100


#define DE              LATJbits.LATJ4                         //485使能


extern uint8_t Baud_n;

struct uart_t {
	uint8_t		len_rx;				// 收字节长度
	uint8_t		len_tx;				// 发字节长度
	uint8_t		cnt_rx;				// 收计数
	uint8_t		cnt_tx;				// 发计数
	uint8_t		buf_rx[MAX_BUF];	// 收缓冲
	uint8_t		buf_tx[MAX_BUF];	// 发缓冲
};

extern struct uart_t gUart;

extern void baudrate_set (uint8_t rate);


extern extern void uart_init (void);

extern void RxCom_Deal (void);


#endif

