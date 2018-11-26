#include <PIC18F4321.h> //include necessary header
#include <stdio.h> //used for string comparison
#include <string.h>

#pragma config OSC = INTIO2 //usual setup
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOR = OFF

#define BUTTON PORTBbits.RB0 //so i don't have to type this every time, take that mplab
#define DELAYTIME 5

void delayOneSec(); //not actually one second, delay to determine short/long
void displayStartup(); //just runs startup commands to lcd
void cmd(unsigned char val); //send command to lcd screen
void data(unsigned char val); //send data to lcd screen
void delay(unsigned int time); //variable delay

int MakeNumber(unsigned char first, unsigned char second); //converts morse to values for the lookup table

void __interrupt() findChar(void); //interrupt to take our assembled morse word and convert to character

unsigned char currentWord[4]; //holds our shorts and longs, morse word
int count; //keeps the currentWord iterating
unsigned char finalChar; //holds the calculated character

unsigned char MorseDecrypt[7][7] = { //lookup table for morse code letter
{'?','?','?','?','?','?','E'},
{'S','H','F','V','?','U','I'},
{'D','B','C','X','Y','K','N'},
{'R','L','P','?','J','W','A'},
{'G','Z','?','Q','?','O','M'},
{'?','?','?','?','?','?','T'},
{'?','?','?','?','?','?','?'}};

int main(void) {
    
    TRISB = 0xFF; //PORTB is an input
    TRISC = 0x00; //PORTC is output, the rs/rw/en lines
    TRISD = 0x00; //PORTD is output, the data bus for LCD screen
    
    ADCON1 = 0x0F; //PORTB is digital input
    PORTC = 0x00; //clear the ports
    PORTD = 0x00;
    
    INTCONbits.GIE = 1; //global interrupts enabled
    INTCONbits.TMR0IE = 1; //timer 0 interrupts enabled
    INTCONbits.TMR0IF = 0; //set the interrupt flag to 0
    
    count = 0;
    for(int i = 0; i < 4; i++) { //initialize the word array to 0
        currentWord[i] = '!';
    }
    
    displayStartup(); //start the display up
    
    while(1) {
        if(BUTTON == 0) { //if button not pressed
        
            T0CON = 0x05; //set up the timer for the idle countdown
            TMR0H = 0xD0; //FF for testing
            TMR0L = 0x00; //FF for testing
            
            T0CONbits.TMR0ON = 1; //start the timer
            
            while(INTCONbits.T0IF == 0) { //while the counter is still ticking...
                if(BUTTON == 1) { //if the button is pressed we stop the timer since we aren't idle anymore
                    break;
                }
            }
            
        } else if(BUTTON == 1) { //if the button is pressed
            T0CONbits.TMR0ON = 0; //stop the idle timer
            
            delayOneSec(); //wait around 1 second
            
            if(BUTTON == 0) { //if the button isn't pressed, we count it as a short
                data('.'); //push short to display
                currentWord[count] = '.'; //hold the short in our word
            } else if (BUTTON == 1) { //if button is pressed, we count it as a long
                data('_');
                currentWord[count] = '_';
                
                while(BUTTON == 1); //hold here until the button is released to prevent extra presses
            }
            
            count++; //increment count by one to help form the word
            
        }
    }
}

void __interrupt() findChar(void) { //this fires if the idle timer completes
    T0CONbits.TMR0ON = 0; //stop the timer
    INTCONbits.T0IF = 0;
    
    T0CON = 0x05; //reset timer
    TMR0H = 0xD0; //CHANGE FOR REAL
    TMR0L = 0x00; //CHANGE FOR REAL
    
    finalChar = MorseDecrypt[MakeNumber(currentWord[0], currentWord[1])][MakeNumber(currentWord[2], currentWord[3])];
    
    cmd(0x01); //clear display
    delay(DELAYTIME);
    
    data(finalChar);
    
    count = 0; //reset count
    
    for(int i = 0; i < 4; i++) { //reset our current word
        currentWord[i] = '!';
    }
    
    T0CONbits.TMR0ON = 1;
}

int MakeNumber(unsigned char first, unsigned char second){ //converts
     unsigned char third [3];
     third[0] = first;
     third[1] = second;
     third[2] = '\0';
     
    if (strcmp(third, ".!") == 0) 
    {
        return 0;
    } 
    else if (strcmp(third, "..") == 0)
    {
        return 1;
    }
    else if (strcmp(third, "_.") == 0)
    {
        return 2;
    }
    else if (strcmp(third, "._") == 0)
    {
        return 3;
    }
    else if (strcmp(third, "__") == 0)
    {
        return 4;
    }
    else if (strcmp(third, "_!") == 0)
    {
        return 5;
    }

    else /* default: */
    {
             return 6;
    }
}

void delayOneSec() { //approximately one second
    for(int i = 0; i < 35; i++) { //i = 300 for testing, 50 for real
        for(int j = 0; j < 255; j++);
    }
}

void displayStartup() {
    delay(DELAYTIME);
    cmd(0x0C); //initialize
    
    delay(DELAYTIME);
    cmd(0x01); //clear display
    
    delay(DELAYTIME);
    cmd(0x06);
    
    delay(DELAYTIME);
    cmd(0x80);
    
    delay(DELAYTIME);
}

void cmd(unsigned char val) {
    PORTD = val;
    PORTC = 0x04;
    delay(DELAYTIME);
    PORTC = 0x00;
}

void data(unsigned char val) {
    PORTD = val;
    PORTC = 0x05;
    delay(DELAYTIME);
    PORTC = 0x01;
}

void delay(unsigned int time) {
    unsigned int i,j;
    for(i = 0; i < time; i++) {
        for(j=0; j<255;j++);
    }
}
