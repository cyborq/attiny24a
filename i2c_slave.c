#include <avr/io.h>
#include <avr/interrupt.h>
#undef F_CPU
#define F_CPU 8000000UL
#include <util/delay.h>

#include "i2c_slave.h"
#include "led.h"

/*! Static Variables
 */

static unsigned char TWI_slaveAddress;
static volatile unsigned char USI_TWI_Overflow_State;


/*! Local variables
 */
static uint8_t TWI_RxBuf[TWI_RX_BUFFER_SIZE];
static volatile uint8_t TWI_RxHead;
static volatile uint8_t TWI_RxTail;

volatile uint8_t TWI_TxBuf[TWI_TX_BUFFER_SIZE];// = {0,0,0,0,0,0,0,0};
static volatile uint8_t TWI_TxHead;
static volatile uint8_t TWI_TxTail;
/*! \brief Flushes the TWI buffers
 */
void Flush_TWI_Buffers(void)
{
    TWI_RxTail = 0;
    TWI_RxHead = 0;
    TWI_TxTail = 0;
    TWI_TxHead = 0;
}

//********** USI_TWI functions **********//

/*! 
 * Initialise USI for TWI Slave mode.
 */
void USI_TWI_Slave_Initialise( unsigned char TWI_ownAddress )
{
  Flush_TWI_Buffers();

  TWI_slaveAddress = TWI_ownAddress;

  PORT_USI |=  (1<<PORT_USI_SCL);                                 // Set SCL high
  PORT_USI |=  (1<<PORT_USI_SDA);                                 // Set SDA high
  DDR_USI  |=  (1<<PORT_USI_SCL);                                 // Set SCL as output
  DDR_USI  &= ~(1<<PORT_USI_SDA);                                 // Set SDA as input
  USICR    =  (1<<USISIE)|(0<<USIOIE)|                            // Enable Start Condition Interrupt. Disable Overflow Interrupt.
              (1<<USIWM1)|(0<<USIWM0)|                            // Set USI in Two-wire mode. No USI Counter overflow prior
                                                                  // to first Start Condition (potentail failure)
              (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|                // Shift Register Clock Source = External, positive edge
              (0<<USITC);
  USISR    = 0xF0;                                                // Clear all flags and reset overflow counter
}


/*! \brief Puts data in the transmission buffer, Waits if buffer is full.
*/
char USI_TWI_Get_Char()
{
	if(TWI_TxHead == TWI_TxTail) return 0;

	TWI_TxTail = (TWI_TxTail +1 ) & TWI_TX_BUFFER_MASK;
	return TWI_TxBuf[TWI_TxTail];
}

void USI_TWI_Transmit_Byte( unsigned char data )
{
	//uint8_t tmp_head;
	//unsigned char data;
	
    unsigned char tmphead;
	//int tmphead =0;
    	tmphead = ( TWI_TxHead + 1 ) & TWI_TX_BUFFER_MASK;  
       // Calculate buffer index.
    while ( tmphead == TWI_TxTail );                           // Wait for free space in buffer.
    TWI_TxBuf[tmphead] = data;                                 // Store data in buffer.
    TWI_TxHead = tmphead;     
//	zapalDiode(1);
	//_delay_ms(50);
//	zgasDiode(1);   
                             // Store new index.
}

/*! \brief Returns a byte from the receive buffer. Waits if buffer is empty.
 */
unsigned char USI_TWI_Receive_Byte( void )
{
    unsigned char tmptail;
    unsigned char tmpRxTail;                                  // Temporary variable to store volatile
    tmpRxTail = TWI_RxTail;                                   // Not necessary, but prevents warnings
    while ( TWI_RxHead == tmpRxTail );
    tmptail = ( TWI_RxTail + 1 ) & TWI_RX_BUFFER_MASK;        // Calculate buffer index
    TWI_RxTail = tmptail;                                     // Store new index
    return TWI_RxBuf[tmptail];                                // Return data from the buffer.
}

/*! \brief Check if there is data in the receive buffer.
 */
unsigned char USI_TWI_Data_In_Receive_Buffer( void )
{
    unsigned char tmpRxTail;                            // Temporary variable to store volatile
    tmpRxTail = TWI_RxTail;                             // Not necessary, but prevents warnings
    return ( TWI_RxHead != tmpRxTail );                 // Return 0 (FALSE) if the receive buffer is empty.
}

/*! \brief Usi start condition ISR
 * Detects the USI_TWI Start Condition and intialises the USI
 * for reception of the "TWI Address" packet.
 */

//#pragma vector=USI_START_VECTOR
// void interrupt USI_Start_Condition_ISR(void)
ISR(USI_START_VECTOR)
{
    unsigned char tmpUSISR;                                         // Temporary variable to store volatile
    tmpUSISR = USISR;                                               // Not necessary, but prevents warnings
// Set default starting conditions for new TWI package
    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
    DDR_USI  &= ~(1<<PORT_USI_SDA);                                 // Set SDA as input
    while ( (PIN_USI & (1<<PORT_USI_SCL)) & !(tmpUSISR & (1<<USIPF)) );   // Wait for SCL to go low to ensure the "Start Condition" has completed.
     _delay_us(4) ;                                                                 // If a Stop condition arises then leave the interrupt to prevent waiting forever.
    USICR   =   (1<<USISIE)|(1<<USIOIE)|                            // Enable Overflow and Start Condition Interrupt. (Keep StartCondInt to detect RESTART)
                (1<<USIWM1)|(1<<USIWM0)|                            // Set USI in Two-wire mode.
                (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|                // Shift Register Clock Source = External, positive edge
                (0<<USITC);
    USISR  =    (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Clear flags
                (0x00<<USICNT0);                                     // Set USI to sample 8 bits i.e. count 16 external pin toggles.
}


/*! \brief USI counter overflow ISR
 * Handels all the comunication. Is disabled only when waiting
 * for new Start Condition.
 */
//#pragma vector=USI_OVERFLOW_VECTOR
// void interrupt USI_Counter_Overflow_ISR(void)
ISR(USI_OVERFLOW_VECTOR)
{
  unsigned char tmpTxTail;     // Temporary variables to store volatiles
  unsigned char tmpUSIDR;


  switch (USI_TWI_Overflow_State)
  {
    // ---------- Address mode ----------
    // Check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else reset USI.
    case USI_SLAVE_CHECK_ADDRESS:
	//zapalDiode(5);
      if ((USIDR == 0) || (( USIDR>>1 ) == TWI_slaveAddress))
      {
        if ( USIDR & 0x01 )
          {
		  USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
		 // zapalDiode(5);
		  SET_USI_TO_SEND_ACK(); //misio
		  }
        else
          USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
          SET_USI_TO_SEND_ACK();
      }
      else
      {
        SET_USI_TO_TWI_START_CONDITION_MODE();
      }
	   _delay_us(4) ;  
      break;

    // ----- Master write data mode ------
    // Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
    case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
//	zapalDiode(4);
      if ( USIDR ) // If NACK, the master does not want more data.
      {
        SET_USI_TO_TWI_START_CONDITION_MODE();
        return;
      }
	  else
	  {
		  USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
	  }
      // From here we just drop straight into USI_SLAVE_SEND_DATA if the master sent an ACK

    // Copy data from buffer to USIDR and set USI to shift byte. Next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
    case USI_SLAVE_SEND_DATA:
		//zapalDiode(1);
	//	_delay_ms(50);
	//	zgasDiode(1);
      // Get data from Buffer
      tmpTxTail = TWI_TxTail;           // Not necessary, but prevents warnings
      if ( TWI_TxHead != tmpTxTail )
      {
        TWI_TxTail = ( TWI_TxTail + 1 ) & TWI_TX_BUFFER_MASK;
		//USIDR = 127;
		 _delay_us(4) ;  
        USIDR = TWI_TxBuf[TWI_TxTail];
      }
      else // If the buffer is empty then:
      {
          SET_USI_TO_TWI_START_CONDITION_MODE();
         // return;
      }
      USI_TWI_Overflow_State = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
      SET_USI_TO_SEND_DATA();
	   _delay_us(4) ;  
      break;

    // Set USI to sample reply from master. Next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
    case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
      USI_TWI_Overflow_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
      SET_USI_TO_READ_ACK();
	   _delay_us(4) ;  
      break;

    // ----- Master read data mode ------
    // Set USI to sample data from master. Next USI_SLAVE_GET_DATA_AND_SEND_ACK.
    case USI_SLAVE_REQUEST_DATA:
      USI_TWI_Overflow_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
      SET_USI_TO_READ_DATA();
      break;

    // Copy data from USIDR and send ACK. Next USI_SLAVE_REQUEST_DATA
    case USI_SLAVE_GET_DATA_AND_SEND_ACK:
      // Put data into Buffer
      tmpUSIDR = USIDR;             // Not necessary, but prevents warnings
      TWI_RxHead = ( TWI_RxHead + 1 ) & TWI_RX_BUFFER_MASK;
      TWI_RxBuf[TWI_RxHead] = tmpUSIDR;

      USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
      SET_USI_TO_SEND_ACK();
      break;
  }
}
