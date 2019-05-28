/*
LED1 - PA3 
LED2 - PA2
LED3 - PA1
LED4 - PA0
ADC - PA7
zasilanie - PB0 zasilanie cał♥ego układu 


*/
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)

#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)

#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)

#if		defined(__AVR_ATtiny24A__)

#define LED_75 A
#define L_75 3
#define LED_50 A
#define L_50 2
#define LED_25 A
#define L_25 1
#define LED_0 A
#define L_0 0
#define ERROR B
#define  ErrD 2


#define PORT_SDA A
#define PIN_SDA 6
#define  PORT_SCL A
#define  PIN_SCL 4
#endif

#define NaladowanaBateriaWPelni 4
#define  NaladowanaBateria 3
#define SrednioNaladowana 2
#define BliskaRozladowania 1
//funkcje
void AC_init();
int AC();
void DiodaNaladowana(int ac);

//tutaj do obsługi i2c
//void I2C_init(uint8_t address);

//tutej do obsługi Errora



//tutej do obslugi diod
void diodyInit();
void zapalDiode(int dioda);
void zgasDiode(int dioda);