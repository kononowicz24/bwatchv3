#ifndef __HM11_H_
#define __HM11_H_

char MAC[]="abcdef123456";

void hm11_init() {
    //DDRC |= 0b00110000; //in idle input pullup
    DDRC &= 0b11001111;
    PORTC |= 0b00110000; //setting Button and Reset in idle

    
    PCMSK3 |= 0b00000001; //set PCINT24 - UART0 RXD
    PCICR |= 0x08; //enable PCINT bank 3
}
void hm11_disconnect() {
    DDRC |= 0b00100000;
    PORTC &= 0b11011111;//PC5 low - BUTTON = 0
    _delay_ms(1000);
    DDRC &= 0b11011111;
    PORTC |= 0b00100000;//PC5 high - BUTTON = 1
    DDRD &= 0b11111110;
}
void hm11_sleep() {
    hm11_disconnect();
    _delay_ms(100);
    Serial.write("AT+SLEEP\r\n");
    _delay_ms(100);
}

#endif