#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "oled.h"
#include "lpc17xx_ssp.h"


#define JOYSTICK_UP 1
#define JOYSTICK_DOWN 2
#define JOYSTICK_LEFT 4
#define JOYSTICK_RIGHT 8

// #define JOYSTICK_UP_OFFSET 3
// #define JOYSTICK_DOWN_OFFSET 15
// #define JOYSTICK_LEFT_OFFSET 4
// #define JOYSTICK_RIGHT_OFFSET 16

#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 64


typedef struct
{
	uint8_t x;
	uint8_t y;
}point_on_screen;

uint8_t read_joystick_direction_code();




static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

}

int main()
{
	int i;
	uint8_t *value_on_axis;
	bool is_direction_positive;
	int snake_move_speed = 1;
	
    init_ssp();
	joystick_init();
	oled_init();
	
    oled_clearScreen(OLED_COLOR_WHITE);

	point_on_screen current_point = {48,32};

	//dzięki tym linijkom na starcie wąż będzie sobie pełzał w prawo. I wskaźnik nie będzie na NULL.
	value_on_axis = &current_point.x; // NIE WIEM, CZY TU MA BYĆ STRZAŁKA CZY KROPKA
	is_direction_positive = true;
	
	while(1)
	{
	
	// sprawdzamy kierunek, w którym będzie się poruszał teraz wąż
	
		joystick_direction_code = read_joystick_direction_code();

			if(joystick_direction_code == JOYSTICK_UP)
			{
				// wąż będzie szedł w górę
				value_on_axis = &current_point.y;
				is_direction_positive = true;
			}
			else if(joystick_direction_code == JOYSTICK_DOWN)
			{
				// wąż będzie szedł  w dół
				value_on_axis = &current_point.y;
				is_direction_positive = false;
			}
			else if(joystick_direction_code == JOYSTICK_LEFT)
			{
				// wąż będzie szedł  w lewo
				value_on_axis = &current_point.x;
				is_direction_positive = false;
			}
			else if(joystick_direction_code == JOYSTICK_RIGHT)
			{
				// wąż będzie szedł  w prawo
				value_on_axis = &current_point.x;
				is_direction_positive = true;
			}
			
		

		
		// zwiększamy lub zmniejszamy odpowiedni element struktury
		if(is_direction_positive)* value_on_axis++;
		else *value_on_axis--;
		
		//sprawdzamy, czy nie wyszliśmy poza ekran
		if(current_point.x>SCREEN_WIDTH) *value_on_axis.x--; // TU TEŻ NIE WIEM, CZY TU MA BYĆ STRZAŁKA CZY KROPKA
		else if(current_point.x<0) *value_on_axis.x++;
		else if(current_point.y>SCREEN_HEIGHT) *value_on_axis.y--;
		else if(current_point.y<0) *value_on_axis.y++;
		
		// dorzucamy nasz punkcik na ekranie
		oled_putPixel(current_point.x,current_point.y, OLED_COLOR_BLACK);

		
		//poczekaj chwilę, żeby ludzkie oko mogło nadążyć
		for(i = 0; i<(100000/snake_move_speed); i++){};

	}

}


uint8_t read_joystick_direction_code()
{
	uint8_t joystick_direction_code = 0; // kierunek wychylenia joysticka zależy od tego, który bit w tej zmiennej zostanie zmieniony, czyli de facto czy ta liczba będzie równa 1,2,4 czy 8

	// wiemy, że piny up i left są podłączone do portu 2, a piny

	uint32_t port0_content = GPIO_ReadValue(0);
	uint32_t port2_content = GPIO_ReadValue(2);

	if((port2_content & (1 << JOYSTICK_UP_OFFSET)) == 0)
	{
		joystick_direction_code = JOYSTICK_UP;
	}
	else if((port0_content & (1 << JOYSTICK_DOWN_OFFSET)) == 0)
	{
		joystick_direction_code = JOYSTICK_DOWN;
	}
	else if((port2_content & (1 << JOYSTICK_LEFT_OFFSET)) == 0)
	{
		joystick_direction_code = JOYSTICK_LEFT;
	}
	else if((port0_content & (1 << JOYSTICK_RIGHT_OFFSET)) == 0)
	{
		joystick_direction_code = JOYSTICK_RIGHT;
	}

	return joystick_direction_code;

}

