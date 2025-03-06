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


 Break			>=12ms
 Marking 		>=8.33ms
*/


#define BAUD_RATE 		1200
#define BIT_TIME		770 // 833us per bit but 800 used to take in account of ~20-40uS error margin on clock
#define BREAK			13 //us

#define SDI12_PIN		NRF_GPIO_PIN_MAP(0, 13)

void sdi12_input() {
	printk("MODESET: Input\n");
	nrf_gpio_cfg_input(SDI12_PIN, GPIO_PIN_CNF_PULL_Pulldown);
}
void sdi12_output() {
	printk("MODESET: Output\n");
	nrf_gpio_cfg_output(SDI12_PIN);
	nrf_gpio_pin_clear(SDI12_PIN);
}

void sdi12_send_break(){
	sdi12_output();
	nrf_gpio_pin_set(SDI12_PIN);
	//printk("12ms sleep start\n");
	k_msleep(BREAK);
	nrf_gpio_pin_clear(SDI12_PIN);
	//printk("8.3ms sleep start\n");
	k_msleep(9); // >=8.3ms break needed before sending command
}

void sdi12_send_char(char c){
	uint8_t bits[10];
	bool parity = 0;
	bits[0] = 1;
	
	for (int i = 0; i < 7; i++){
		bits[i + 1] = !((c >> i) & 1);
		if(bits[i + 1] == 1) parity = !parity;
	}
	bits[8] = parity;
	bits[9] = 0;

	
	for (int i = 0; i < 10; i++){
		
		bits[i] == 0 ? nrf_gpio_pin_clear(SDI12_PIN) : nrf_gpio_pin_set(SDI12_PIN);
		bits[i] == 0 ? printk("0") : printk("1");
		k_busy_wait(BIT_TIME);
	}
	//printk("\n");
}


void sdi12_send_command(const char* cmd){
	sdi12_send_break();
	while(*cmd){
		sdi12_send_char(*cmd++);
	}
}

void sdi12_wait_start_bit() {
	// TODO: set timeout
	while(nrf_gpio_pin_read(SDI12_PIN) == 0);
}

uint8_t* sdi12_read_response(uint8_t* buffer, size_t buffer_size) {
	sdi12_input();

	uint8_t byte = 0;
	uint8_t char_value;
	size_t char_idx = 0;
	int count = 0;
	while(1){
		sdi12_wait_start_bit();
		
		k_busy_wait(BIT_TIME / 2);
		for(int i = 0; i < 8; i++){
			byte >>= 1;

			if(nrf_gpio_pin_read(SDI12_PIN) == 0){
				byte |= 0b10000000;
				printk("1");
			} else{
				byte &= 0b01111111;
				printk("0");
			}
			k_busy_wait(BIT_TIME);
		}
		printk("\n");

		// TODO: Check stop bit validity

		// TODO: Check parity bit

		// TODO: Check for bufferoverflow

		char_value = byte & 0b01111111;
		if(char_value == '\r'){
			break;
		}

		buffer[char_idx++] = char_value;
		if(count == 30) break;
		count++;
		
	}
	buffer[char_idx] = '\0';

	return buffer;
}

int main(void){
	sdi12_output();
	uint8_t buffer[150];

	printk("Waiting 10 seconds\n");
	k_msleep(10000);
	printk("-------- Sending command 0I! ---------\n");
	sdi12_send_command("0M!\r\n");
	uint8_t* result = sdi12_read_response(buffer, sizeof(buffer));
	k_msleep(13000);
	sdi12_send_command("0D0!\r\n");
	uint8_t* result2 = sdi12_read_response(buffer, sizeof(buffer));
	/*
	for(int i = 0; i<50; i++){
		printk("%d", result[i]);
	}
	*/

	printk("%s", result2);
	
	return 0;
}