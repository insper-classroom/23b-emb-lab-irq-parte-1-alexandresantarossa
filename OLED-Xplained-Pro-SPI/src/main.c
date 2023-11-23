/************************************************************************
 * 5 semestre - Eng. da Computao - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Material:
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 *
 * Objetivo:
 *  - Demonstrar interrupção do PIO
 *
 * Periféricos:
 *  - PIO
 *  - PMC
 *
 * Log:
 *  - 10/2018: Criação
 ************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Bot�es
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX  28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);

/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

/* flag */
volatile char but_flag; // (1)

/* funcao de callback/ Handler */
void but_callback(void){
	but_flag = 1;
}

/************************************************************************/
/* fun��es                                                              */
/************************************************************************/

// pisca led N vez no periodo T
void pisca_led(int n, int t){
  for (int i=0;i<n;i++){
    pio_clear(LED_PIO, LED_IDX_MASK);
    delay_ms(t);
    pio_set(LED_PIO, LED_IDX_MASK);
    delay_ms(t);
  }
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{
	
  // Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

  // Inicializa clock do perif�rico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

  // Configura PIO para lidar com o pino do bot�o como entrada
  // com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);

  // Configura interrup��o no pino referente ao botao e associa
  // fun��o de callback caso uma interrup��o for gerada
  // a fun��o de callback � a: but_callback()
  pio_handler_set(BUT_PIO,
                  BUT_PIO_ID,
                  BUT_IDX_MASK,
                  PIO_IT_RISE_EDGE,
                  but_callback);

  // Ativa interrup��o e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
  pio_get_interrupt_status(BUT_PIO);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais pr�ximo de 0 maior)
  NVIC_EnableIRQ(BUT_PIO_ID);
  NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/



// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	sysclk_init();
	board_init();
	
	int delay = 2000;
	char freq[128];
	
	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	 // configura botao com interrupcao
	io_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();
	
	while(1) {
		gfx_mono_draw_string(freq,55,20,&sysfont);
		
		if (but_flag) {  // (2)
			delay_ms(delay);
			if (pio_get(BUT_PIO,PIO_INPUT,BUT_IDX_MASK)) {
				delay -= 200;
			}
			else {
				delay += 200;
			}

			pisca_led(5,delay); // (3)
			but_flag = 0;
		   }	
		   
		   sprintf(freq, "%d", delay);
		   pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
	
