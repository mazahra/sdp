/**********************************************************************
 Module
   Xbee.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
 12-29-12 2:10  PM      jash        Created file.
 1-17-13  4:10  PM      jash        Work on functions, add receieve pseudo code
 2-1-13   2:50  AM      jash        Complete functions and add comments.
 2-9-13   5:08  PM      jash        Added Mavlink functionality
 2-9-15   12:40 PM      jash        MAVLink test up and running, and set channel
***********************************************************************/

#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include <ports.h>
#include "Board.h"
#include "Uart.h"
#include "Mavlink.h"
#include "Timer.h"
#include "Xbee.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

//#define XBEE_TEST //used for testing Xbee
#define API_DELAY           1000
// note, need to reprogram Xbee for different Baud Rates.
//  factory settings are 9600 baud rate
#define XBEE_BAUD_RATE      9600

/*    FOR IFDEFS     */
//#define XBEE_REPROGRAM_SETTINGS
//#define UNICAST_MSG


/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static uint8_t programMode();

/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/
static uint8_t xbeeUartId;

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

uint8_t Xbee_init(uint8_t uartId){
    xbeeUartId = uartId;
    UART_init(uartId,XBEE_BAUD_RATE);
#ifdef XBEE_REPROGRAM_SETTINGS
    if( programMode() == FAILURE){
        while(1);
        return FAILURE;
    }
#else
    /*
    int i = 0;
    char confirm[3];
    DELAY(2000);
    UART_putString(uartId, "+++", 3);
    DELAY(API_DELAY);
    //wait for "OK\r"
    do {
        confirm[i] = UART_getChar(XBEE_UART_ID);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);

    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
     * */
#endif
    return SUCCESS; 
}


void Xbee_runSM(){
    //Recieve bytes if they are available
    if(UART_isReceiveEmpty(xbeeUartId) == FALSE){
        Mavlink_recieve(xbeeUartId);
    }
}

uint8_t Xbee_getUartId() {
    return xbeeUartId;
}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

/**********************************************************************
 * Function: programMode()
 * @return Success or Failure based on weather the mode could be set.
 * @remark Currently restores the XBEE to factory setting
 * @author John Ash
 * @date February 10th 2013
 **********************************************************************/
#ifdef XBEE_REPROGRAM_SETTINGS
static uint8_t programMode(){
    int i = 0;
    char confirm[3];

    DELAY(API_DELAY);
    DELAY(API_DELAY);

    UART_putString(xbeeUartId, "+++", 3);
    DELAY(API_DELAY);
    //wait for "OK\r"
    do {
        confirm[i] = UART_getChar(xbeeUartId);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);

    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATRE\r", 5);// Resets to Factory settings
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATCH15\r", 7);
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATDH0\r", 6);
    DELAY(API_DELAY);
    #ifdef UNICAST_MSG
    #ifdef IS_COMPAS
    UART_putString(xbeeUartId, "ATDLAAC3\r", 9);
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATMYBC64\r", 9);
    #else
    UART_putString(xbeeUartId, "ATDLBC64\r", 9);
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATMYAAC3\r", 9);
    #endif
    DELAY(API_DELAY);
    #endif
    UART_putString(xbeeUartId, "ATWR\r", 5);//Writes the command to memory
    DELAY(API_DELAY);
    UART_putString(xbeeUartId, "ATCN\r", 5);//Leave the menu.
    return SUCCESS;
}
#endif


/*************************************************************
 * This test function will program two Xbees, Master & Slave.
 * The Master will send data packets 1-255. However before
 * sending the next packet in sequence it has to recieve a
 * return packet from Slave.
 *
 * Slave will look for packets, and then return the packet data
 * field in  a packet to the Master
 */
#ifdef XBEE_TEST

#include "Serial.h"

#define TIMER_TIMEOUT 4
#define DELAY_TIMEOUT 1000
#define TIMER_STATUS 3
#define DELAY_STATUS 4000

#define XBEE_UART_ID    UART2_ID

uint32_t count_recieved = 0, count_lost = 0;

int main(){
    Board_init();
    Serial_init();
    printf("Welcome to Xbee Test1\n");
    printf("UART INIT\n");
    Timer_init();

    printf("Timers Init\n");
    if(Xbee_init(XBEE_UART_ID) == FAILURE){
        printf("Xbee Failed to initilize\n");
        return FAILURE;
    }
    printf("XBEE Initialized\n");

// Master sends packets and listens for responses
    #ifdef XBEE_1
    Mavlink_send_Test_data(XBEE_UART_ID, 1);
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
    Timer_new(TIMER_STATUS, DELAY_STATUS);
    while(1){
        Xbee_runSM();
        //lost a packet, report it, and restart
        if(Timer_isActive(TIMER_TIMEOUT) != TRUE){
            Mavlink_send_Test_data(XBEE_UART_ID, 1);
            count_lost++;
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
            printf("lost_packet: %d\n", get_time());
        }
        //Printout the status
        if(Timer_isActive(TIMER_STATUS) != TRUE){
            Timer_new(TIMER_STATUS, DELAY_STATUS);
            printf("Status: %d,%d [Recieved,Lost] TIME: %d\n", count_recieved, count_lost, get_time());
        }

    }
    #else
    while(1){
        Xbee_runSM();
    }
    #endif
}


void Xbee_message_data_test(mavlink_test_data_t* packet){
    Mavlink_send_Test_data(Xbee_getUartId(), (packet->data+1)%255);
    count_recieved++;
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
}

#endif


//#define XBEE_TEST_2
#ifdef XBEE_TEST_2
#define XBEE_UART_ID    UART2_ID

#define TIMER_TIMEOUT 4
#define DELAY_TIMEOUT 4000

int main(){
    Board_init();
    Serial_init();
    Timer_init();
    Xbee_init(XBEE_UART_ID);
    printf("Xbee Test 2\n");
    while(1){
        Xbee_runSM();
        //if(!UART_isTransmitEmpty(UART1_ID));
        //printf("%d\t",Mavlink_returnACKStatus(messageName_start_rescue));
        if(!UART_isReceiveEmpty(UART1_ID)){
            Serial_getChar();
            Mavlink_send_start_rescue(UART2_ID, TRUE, 0xFF, 0x34FD, 0xAB54);
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
            printf("\nSENT\n");
        }
        else if(Timer_isActive(TIMER_TIMEOUT) != TRUE && Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_WAIT){
        //else if(Timer_isActive(TIMER_TIMEOUT) != TRUE){
            Mavlink_editACKStatus(messageName_start_rescue, ACK_STATUS_DEAD);
            printf("ACK DEAD\n");
        }
        else if(Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_DEAD){
            Mavlink_send_start_rescue(UART2_ID, TRUE, 0xFF, 0x34FD, 0xAB54);
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
        }
        else if(Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_RECIEVED){
            Mavlink_editACKStatus(messageName_start_rescue, ACK_STATUS_NO_ACK);
            printf("GPS SENT AND ACKOWLEGED\n");
        } 
    }
}

#endif

//#define XBEE_TEST_3
#ifdef XBEE_TEST_3

#define XBEE_UART_ID UART2_ID

#define STARTUP_DELAY   1500
#define PRINT_DELAY     1000

int main(){
    Board_init();
    Board_configure(USE_SERIAL | USE_TIMER);
    //Board_configure(USE_LCD | USE_TIMER);
    DELAY(10);
    dbprint("Starting XBee...\n");
    if (Xbee_init(XBEE_UART_ID) != SUCCESS) {
        dbprint("Failed XBee init.\n");
        return FAILURE;
    }
    dbprint("XBee initialized.\n");
    DELAY(STARTUP_DELAY);

    LCD_setPosition(0,0);
    Timer_new(TIMER_TEST, PRINT_DELAY);
    unsigned long int sent = 0;
    unsigned long int got = 0;
    LCD_clearDisplay();
    LCD_setPosition(2,0);
    dbprint("Received: %ld", got);
    while(1){
        Xbee_runSM();
        if (Timer_isExpired(TIMER_TEST)) {
            LCD_setPosition(0,0);
            dbprint("Sent XBee msg.\nSent: %ld\n", ++sent);
            Mavlink_sendSaveStation(WANT_ACK);
            Timer_new(TIMER_TEST, PRINT_DELAY);
        }
        if (Mavlink_hasNewMessage()) {
            LCD_setPosition(2,0);
            dbprint("Received: %ld", ++got);
        }
    }

    return SUCCESS;
}
#endif


//#define XBEE_ATLAS_TEST
#ifdef XBEE_ATLAS_TEST

#define XBEE_UART_ID UART1_ID

#define STARTUP_DELAY   1500
#define PRINT_DELAY     1000

int main(){
    Board_init();
    Board_configure(USE_TIMER);
    DELAY(10);
    //dbprint("Starting XBee...\n");
    if (Xbee_init(XBEE_UART_ID) != SUCCESS) {
        //dbprint("Failed XBee init.\n");
        return FAILURE;
    }
    //dbprint("XBee initialized.\n");
    DELAY(STARTUP_DELAY);

    Timer_new(TIMER_TEST, PRINT_DELAY);
    unsigned long int sent = 0;
    unsigned long int got = 0;
    while(1){
        Xbee_runSM();
        if (Timer_isExpired(TIMER_TEST)) {
            //dbprint("XBee: Sent=%ld, Got=%ld\n", ++sent, got);
            Mavlink_sendRequestOrigin();
            Timer_new(TIMER_TEST, PRINT_DELAY);
        }
        if (Mavlink_hasNewMessage()) {
            ++got;
        }
    }

    return SUCCESS;
}
#endif
