#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <hal/nrf_gpio.h>



/*
***SDI-12 Configuration TEROS 12***
Baud Rate 		1200
Start Bits		1
Data Bits		7 (LSB first)
Parity Bits		1
Stop Bits		1
Logic			Inverted

***SDI-12 Timing***
Command and response are preceded by an address and terminated by a carriage return and line feed
combination <CR><LF>

	START D0 D1	D2 D3 D4 D5 D6 EP STOP
 ___-----____---------______---_______------


 Break			12ms
 Marking 		8.33ms
*/


#define BAUD_RATE 		1200
#define BIT_TIME		(1000000 / BAUD_RATE) // 833us per bit
#define BREAK			12000 //us

#define SDI12_PIN		NRF_GPIO_PIN_MAP(0, 17)

void sdi12_send_break(){
	//nrf_gpio_cfg_output(SDI12_PIN);
	//nrf_gpio_pin_set(SDI12_PIN);
	printk("12ms sleep start\n");
	k_usleep(BREAK);
	//nrf_gpio_pin_clear(SDI12_PIN);
	printk("8.3ms sleep start\n");
	k_usleep(8300); // 8.3ms min break needed before sending command
}

void sdi12_send_char(char c){
	uint8_t bits[10];
	bits[0] = 1;
	for (int i = 0; i < 8; i++){
		bits[i + 1] = (c >> i) & 1;
	}
	bits[9] = 0;

	for (int i = 0; i < 10; i++){
		// bits[i] == 0 ? nrf_gpio_pin_clear(SDI12_PIN) : nrf_gpio_pin_set(SDI12_PIN);
		bits[i] == 0 ? printk("0") : printk("1");
		k_usleep(BIT_TIME);
	}
	printk("\n");
}

void sdi12_send_command(const char* cmd){
	sdi12_send_break();
	while(*cmd){
		sdi12_send_char(*cmd++);
	}
}




int main(void){
	nrf_gpio_cfg_output(SDI12_PIN);

	while(1){
		printk("-------- Sending command 0! ---------\n");
		sdi12_send_command("0!");
		k_sleep(K_SECONDS(10));
	}
	return 0;
}
