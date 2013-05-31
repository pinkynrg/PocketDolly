/* 
 * File:   main.c
 * Author: francescomeli
 *
 * Created on May 1, 2013, 3:43 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
#include "delay.c"

#define WISE                0
#define PIC_CLK             8000000 //8Mhz
#define _XTAL_FREQ          8000000 //8Mhz
#define true                1
#define false               0
#define delay_us(x)         __delay_us(x)
#define delay_ms(x)         __delay_ms(x)
#define delay_s(x)          __delay_s(x)
#define NOP8                asm("nop"); asm("nop"); asm("nop"); asm("nop");asm("nop"); asm("nop"); asm("nop"); asm("nop");
#define NOP4                asm("nop"); asm("nop");asm("nop"); asm("nop");
#define NOP2                asm("nop"); asm("nop");
#define LDOLLY              1000    //dolly length [mm]
#define LSPIN               5       //length 1 spin [mm]
#define TSPIN               500    //time spin

#define MOTOR1              RA0
#define MOTOR2              RA1
#define MOTOR3              RA2
#define MOTOR4              RA3
#define EN                  RA4     //enable display
#define RS                  RA5     //reset (?)
#define IR                  RB1
#define databits            PORTC   //data input, output display
#define RIGHT               RB4
#define LEFT                RB7
#define SELECT              RB6
#define CANCEL              RB5
#define END                 RB0



__CONFIG(CONFIG1 & DEBUG_OFF & LVP_OFF & FCMEN_OFF & IESO_OFF & BOREN_OFF & CPD_OFF & CP_OFF & MCLRE_OFF & PWRTE_OFF & WDTE_OFF & FOSC_INTRC_NOCLKOUT);
__CONFIG(CONFIG2 & WRT_OFF & BOR4V_BOR21V);

const unsigned char menuLengths[4]={3,2,1,3};
const int pulse[4][4]=  {{0,1,1,0},     //06 decimale
                         {1,0,1,0},     //10 decimale
                         {1,0,0,1},     //09 decimale
                         {0,1,0,1}};    //05 decimale

unsigned char wise;                     //decleared to manage interrupt service request
    
typedef struct menuItem {
    const char             * voice;
    const char             * mask;         //se esiste value allora questa sara' l-etichetta che accompagna il valore (tipo 60 sec, 60 e' value "sec" e' l-etichetta)
    const unsigned char    up;           //punta al menu superiore
    const unsigned char    indexUp;      //punta all'indice del menu superiore
    const unsigned char    down;         //punta al submenu
    int                    value;        //se esiste, questo e' il valore della voce (tipo velocity=100)
};

typedef struct menuPosition {
    unsigned char menuIndex;          //indice del menu in cui ci troviamo
    unsigned char index;              //indice che punta alla voce del menu in cui siamo
    unsigned char set;                //set e' 1 se sto' settando un valore del menu
};

typedef struct menu {
    int    start;
    struct menuPosition pos;
    struct menuItem * m[4];
};

typedef struct data {
    unsigned int     totalTime;
    float            spins_btw_shots;
    unsigned int     delay_btw_shots;
    unsigned int     totalShots;
    unsigned char    wise;
    unsigned char    actualShots;
};

void            functionTryThings();
void            SendIRPulseCycles(char cycles);
void            waitExactUsHex(char hByte, char lByte);
void            SendIRSequence();
void            pic_init(void);
void            InitRB0Interrupt(void);
void            LCD_STROBE(void);
void            cmd(unsigned char c);
void            clear(void);
void            lcd_init();
void            data(unsigned char c);
void            string(const char *q);
void            print(struct menu myMenu);
void            motorRelax();
void            start(struct data myData);
int             getKey();
struct menu     mainMenu_init(struct menu myMenu);
struct menu     left(struct menu myMenu);
struct menu     right(struct menu myMenu);
struct menu     select(struct menu myMenu);
struct menu     deselect(struct menu myMenu);
struct data     spin(struct data myData);
struct data     getData(struct menu myMenu);
unsigned char   takePic(unsigned char actualShots);
void            printData (struct data myData);



void main(void) {
    int key;
    static struct menuItem mainMenu[3] = {{"Settings","",255,255,1,255},{"Try Features","",255,255,2,255},{"Start","",255,255,254,255}};
    static struct menuItem settings[2] = {{"Total Time","",0,0,3,255},{"Total Shots","",0,0,255,100}};
    static struct menuItem try[1]      = {{"Take a Picture","",0,1,253,255}};
    static struct menuItem time[3]     = {{"Hours","h", 1,0,255,0},{"Minutes","m", 1,0,255,3},{"Seconds","s", 1,0,255,0}};
    struct menu myMenu =                 {0,{0,0,0},{mainMenu,settings,try,time}};
    struct data myData =                 {0,0,0,0,0,0};


    pic_init();
    lcd_init();
    motorRelax();

    //functionTryThings();

    while (1) {
        while(myMenu.start==0) {

            print(myMenu);

            key=getKey();

            switch (key) {
                    case 1: {
                        myMenu=left(myMenu);
                        break;
                    }
                    case 2: {   
                        myMenu=right(myMenu);
                        break;
                    }
                    case 3: {   
                        myMenu=select(myMenu);
                        break;
                    }
                    case 4: {  
                        myMenu=deselect(myMenu);
                        break;
                    }
                    default: {1; break;}
            }
        }
        
        myData=getData(myMenu);     //prendo data

        start(myData);              //uso data per timelapse

        motorRelax();

        myMenu.start=0;             //reset start variable
    }
}

void functionTryThings() {
    while (1) {
       unsigned char c=255;
        while (c--) {
            RC4=0;
            RC4=1;
        }
    }
}

void start(struct data myData) {
    int i;
    printData(myData);

    for (i=0; i<myData.totalShots; i++) {
        DelayS(myData.delay_btw_shots);
        myData=spin(myData);
        myData.actualShots=takePic(myData.actualShots);
        printData(myData);
    }

    DelayS(2); //da il tempo allo user di vedere che il processo e' giunto al termine
}


#define waitExactUs(x) waitExactUsHex(x/256, (x%256)/5);
void waitExactUsHex(char hByte, char lByte)
{
	char i;
	while (lByte--) { continue; }	//delay for the extra bit
	while (hByte--) {				//delay loop for the bulk of the time
		i = 61;
		while (i--) { continue; }
		}
}

#define SendIRPulse(x) SendIRPulseCycles(x/25); //%8 perche la lunghezza di un while(x--) per 8Mhz = 8us
void SendIRPulseCycles(char cycles)
{   
    while (cycles--)		//this loop is exactly 25us - of approximately 50% dutycycle
    {
//      IR = 1;         //modulation for 4Mhz intosc
//      NOP4; NOP2;
//      IR = 0;
        IR = 1;
        NOP8; NOP8; NOP4;
        IR = 0;
        NOP8;
    }
}

void SendIRSequence()
{
	SendIRPulse(2250);	//2250us of modulated IR (0 resto)
        delay_us(27600);	//delay 27600 us         (5us resto)
	SendIRPulse(650);	//650us of modulated IR  (0 resto)
	delay_us(1375);         //delay 1375 us          (5us resto)
	SendIRPulse(575);	//575us of modulated IR  (0 resto)
	delay_us(3350);         //delay 3350 us          (5us resto)
	SendIRPulse(650);	//650us of modulated IR  (0 resto)
}

struct data getData(struct menu myMenu) {
    struct data myData;
    myData.totalTime=(myMenu.m[3][0].value*3600)+(myMenu.m[3][1].value*60)+(myMenu.m[3][2].value*1); //[sec]
    myData.totalShots=myMenu.m[1][1].value;//[unitless]
    myData.spins_btw_shots=(LDOLLY/myData.totalShots)/LSPIN;//[unitless]
    myData.delay_btw_shots=(myData.totalTime/myData.totalShots);//[sec] -(TSPIN*myData.spins_btw_shots); //[] mettere totalTime*1000 in qualche modo
    myData.wise=WISE;
    myData.actualShots=0;
    return myData;
}

void motorRelax(void) {
    MOTOR1=0;
    MOTOR2=0;
    MOTOR3=0;
    MOTOR4=0;
}

struct data spin(struct data myData) {
    int i,s,f;
 
    s = myData.spins_btw_shots*200;    //numbero of total steps

    for(i=0; i<s; i++) {
        DelayMs(TSPIN/200);
        if (myData.wise) f=0; else f=3;
        MOTOR1=pulse[i%4][abs(3-f)];
        MOTOR2=pulse[i%4][abs(2-f)];
        MOTOR3=pulse[i%4][abs(1-f)];
        MOTOR4=pulse[i%4][abs(0-f)];
        if (END) {while (END); if (myData.wise==1) myData.wise=0; else myData.wise=1;}
        //if ((myData.delay_btw_shots>=10)&&(i%10000==0)) SendIRPulse(1000);	//keeps the camera awake con un impulso 4KHz durante 1 secondo se delay tra shots maggiore uguale 10 secondi

    }

    motorRelax();

    return myData;
}

struct menu left(struct menu myMenu) {
    if (myMenu.pos.set==1) {
        if (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value>0)
            myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value--;
    }
    else
        if (myMenu.pos.index>0)
            myMenu.pos.index--;
    return myMenu;
}

struct menu right(struct menu myMenu) {
    if (myMenu.pos.set==1) {
        if (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value<=255)
            myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value++;
    }
    else {
        if (myMenu.pos.index<menuLengths[myMenu.pos.menuIndex]-1)
            myMenu.pos.index++;
    }
    return myMenu;
}

struct menu select(struct menu myMenu) {
    if (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].down>=250) {   //da 250 in poi ci teniamo valori da assegnare a funzioni particolari
        switch (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].down) {
            case 255: {if (myMenu.pos.set==1) myMenu.pos.set=0; else myMenu.pos.set=1; break;}
            case 254: {myMenu.start=true; break;}                               //trigger start
            case 253: {while (1) { takePic(0); delay_ms(2000); } break;}                   //funzione per scattare una foto
        }
    }
    else {
        myMenu.pos.menuIndex=myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].down;
        myMenu.pos.index=0;
    }
    return myMenu;
}

struct menu deselect(struct menu myMenu) {
    if (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].up!=255) {
        if (myMenu.pos.set==1) myMenu.pos.set=0;
        else {
            myMenu.pos.index=myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].indexUp;
            myMenu.pos.menuIndex=myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].up;
        }
    }
    return myMenu;
}

void print (struct menu myMenu) {

    char buffer[16];  /* size of line on display */
    clear();
    if (myMenu.pos.set==0)
        sprintf(buffer,"> %s",myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].voice);
    else sprintf(buffer,"  %s",myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].voice);
        cmd(0x80);
    string(buffer);

    if (myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value!=255) {
        if (myMenu.pos.set==0)
            sprintf(buffer, "  %d %s", myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value,myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].mask);  /* Same formatting options as printf */
        else sprintf(buffer, "> %d %s", myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].value,myMenu.m[myMenu.pos.menuIndex][myMenu.pos.index].mask);
        cmd(0xc0);
        string(buffer);
    }

    delay_us(100);
}

void printData (struct data myData) {
    char buffer[16];
    int  morelessspins=myData.spins_btw_shots;
    clear();
    sprintf(buffer,"Took:%d ToDo:%d",myData.actualShots, myData.totalShots-myData.actualShots);
    cmd(0x80);
    string(buffer);
    sprintf(buffer,"Sp:%d Sc:%d",morelessspins, myData.delay_btw_shots);
    cmd(0xc0);
    string(buffer);
}

void pic_init(void) {

    OSCCON = 0b11110001;    //oscillator control

    ANSELH = 0b00000000;    //TODO: forse non necessarie per pic16f887: controllare
    ANSEL  = 0b00000000;

    TRISA  = 0b00000000;
    TRISB  = 0b11110001;
    TRISC  = 0b00000000;
    TRISD  = 0b00000000;
    TRISE  = 0b00000000;

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;

    //InitRB0Interrupt();
}

void LCD_STROBE(void) {
    EN = 1;
    delay_us(1);
    EN = 0;
}
void cmd(unsigned char c) {
    RS = 0;
    delay_us(50);
    databits = (c >> 4);
    LCD_STROBE();
    databits = (c);
    LCD_STROBE();
    delay_ms(1);

}
void clear(void)
{
    cmd(0x01);
}
void lcd_init(void) {
    cmd(0x38);           // 8bit, multiple line, 5x7 display
    cmd(0x38);           // 8bit, multiple line, 5x7 display
    cmd(0x38);           // 8bit, multiple line, 5x7 display
    cmd(0x28);           // Function set (4-bit interface, 2 lines, 5*7Pixels)
    cmd(0x28);           // Function set (4-bit interface, 2 lines, 5*7Pixels)
    cmd(0x28);           // Function set (4-bit interface, 2 lines, 5*7Pixels)
    cmd(0x0c);           // Make cursorinvisible
    cmd(0x01);           // Clear screen
    cmd(0x6);            // Set entry Mode(auto increment of cursor)
}

void data(unsigned char c)
{
    RS = 1;
    delay_us(50);
    databits = (c >> 4);
    LCD_STROBE();
    databits = (c);
    LCD_STROBE();
}
void string(const char *q)
{
    while (*q) {
        data(*q++);
    }
}

int getKey(void) {
    int btn=0;

    while (btn==0) {
              if (LEFT==1)  {btn=1; while (LEFT==1);}
         else if (RIGHT==1) {btn=2; while (RIGHT==1);}
         else if (SELECT==1){btn=3; while (SELECT==1);}
         else if (CANCEL==1){btn=4; while (CANCEL==1);}
    }

    return btn;
}

unsigned char takePic(unsigned char actualShots) {
    SendIRSequence();	//first sequence
    delay_ms(63);       //actually 63ms
    SendIRSequence();	//second sequence
    actualShots++;
    return actualShots;
}

//void InitRB0Interrupt(void)
//{
//  INTEDG = 0;                                //Falling edge.
//  INTF = 0;                                  //Clear interrupt flag.
//  INTE = 1;                                  //Enable EXT interrupt.
//  //GIE = 1;                                   //Enable all interrupt.
//  return;
//}