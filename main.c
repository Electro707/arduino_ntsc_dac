#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>
#include "image.h"

#define PORT_SET    PORTD
#define EVER        ;;

// ire to voltage, full scale is 100 IRE, zero is -40 IRE
// adc is setup as 8 bits
// volt_level = 256 * ((ire+40)/140)
#define VOLTAGE_BLANKING    73          // ire=0
#define VOLTAGE_SYNC        0           // ire=-40
#define COLOR_BLACK         87          // ire=7.5
#define COLOR_WHITE         255

void horizFontPorch(void);
void verticalSync(void);
void verticalSync2(void);
void horizonalLine(void);
void horizVideoHalf(void);

uint16_t horizN = 1;
uint16_t lineN = 0;          // actual number number

void VSync(void){
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(144);       //
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_1(25);       // delay for 4.7uS, 25*3 instructions
}

void VSyncLine(void){
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(144);       //
    PORT_SET = VOLTAGE_BLANKING;
    _delay_loop_1(25);       // delay for 4.7uS, 25*3 instructions

    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(144);       //
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_1(25);       // delay for 4.7uS, 25*3 instructions
}

void equalization(void){
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(12);       // delay for 2.3uS
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_1(157);       // delay for 29.45uS (half of line with 2.3uS out)
}

void equalizationLine(void){
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(12);       // delay for 2.3uS
    PORT_SET = VOLTAGE_BLANKING;
    _delay_loop_1(157);       // delay for 29.45uS (half of line with 2.3uS out)

    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(12);       // delay for 2.3uS
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_1(157);       // delay for 29.45uS (half of line with 2.3uS out)
}

void horizFontPorch(void){
    // delay of 1.5uS
    PORT_SET = VOLTAGE_BLANKING;
    _delay_loop_1(8);       // delay for 1.5uS, 8*3 instructions
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(25);       // delay for 4.7uS, 25*3 instructions
    PORT_SET = VOLTAGE_BLANKING;
    _delay_loop_1(25);       // delay for 4.7uS, 25*3 instructions
}

// draws 2 lines, as we want a checkmark
void horizonalLine(void){
    asm volatile(
        // horizonal porch, send a blank signal
        "ldi r25, %[cont1]"     "\n\t"
        "out 0x0b, r25"         "\n\t"
        // delay for 1.5uS (24 clock cycles) , let's do some math!
        // take the current line number, then divide by 4 (right shift 2)
        "lds r18, %0"       "\n\t"      // 2 clk
        "lds r19, %0+1"     "\n\t"      // 2 clk (4)
        "lsr r19"           "\n\t"      // 1 clk (5)
        "ror r18"           "\n\t"      // 1 clk (6)
        "lsr r19"           "\n\t"      // 1 clk (7)
        "ror r18"           "\n\t"      // 1 clk (8)
        // now that we shifted out line number by 2, our abs max value is 131 for a 525 lines (not possible, but the max vertical scan lines)
        // so R19 should be zero, which frees us to use it
        "ldi r20, 180"      "\n\t"      // 1 clk (9)
        "mul r18, r20"      "\n\t"      // 2 clk (11)
        // move the contents of r0:r1 back to r18-r19
        "movw r18, r0"      "\n\t"      // 1 clk (12)
        // now we add out image pointer to the image XY counter
        "ldi r30, %1"       "\n\t"      // 1 clk (13)
        "ldi r31, 0"     "\n\t"         // 1 clk (14)
        "add r30, r18"      "\n\t"      // 1 clk (18)
        "adc r31, r19"      "\n\t"      // 1 clk (16)
        // now currImgPtr lives in r30:r31 (Z register)
        // we have 8 instructions left, with two to load value, so 7 NOP
        "NOP\n\t" "NOP\n\t" "NOP\n\t" "NOP\n\t"
        // "NOP\n\t" "NOP\n\t" "NOP\n\t"
        // now we
        "ldi r25, %[cont2]"     "\n\t"
        "out 0x0b, r25"         "\n\t"
        // now we wait for 75 instructions, so we make a loop
        // 1 instruction for loading value, and 1 to exit, 1 after to load next port value, so 72 wait states. 3 cycles each, so 24 instructions
        "ldi r25, 24"     "\n\t"
        "dec r25"         "\n\t"
        "brne .-4"        "\n\t"
        // second blanking pulse
        "ldi r25, %[cont1]"     "\n\t"
        "out 0x0b, r25"         "\n\t"
        // same delay as above
        "ldi r25, 24"     "\n\t"
        "dec r25"         "\n\t"
        "brne .-4"        "\n\t"
        // sending the image itself
        "ldi r25, 100"       "\n\t"
        // "lds r30, %0"       "\n\t"
        // "lds r31, %0+1"     "\n\t"
        "lpm r24, Z+"       "\n\t"
        "out 0x0b, r24"     "\n\t"
        "dec r25"           "\n\t"
        "brne .-8"           "\n\t"
        : // no output
        : "m" (lineN), "m" (image), [cont1] "i" (VOLTAGE_BLANKING), [cont2] "i" (VOLTAGE_SYNC)
    );
}

// draws 2 lines, as we want a checkmark
// void horizonalLine(void){
//     uint8_t n = 65;
//     // each period must take ~52.6uS (total line is ~63.5uS)
//     uint16_t temp = currImgPtr;     // make a copy of the global variable so that C can optimize loop instructions
//
//     horizFontPorch();
//     while(n--){
//         PORT_SET = pgm_read_byte_near(temp);
//         // imageData = pgm_read_byte_near(image + imageXYcounter);
//         // _delay_loop_1(1);      // 14
//         // _NOP();
//         // _NOP();
//         // _NOP();
//         temp++;
//     }
//     currImgPtr = temp;
// }

// void horizonalLine(void){
//     uint8_t n = 8;
//     // for now do black and white titles
//     // each period must take ~52.6uS (total line is ~63.5uS)
//     horizFontPorch();
//     while(n--){
//         PORT_SET = COLOR_BLACK;
//         _delay_loop_1(10);      // 14
//         PORT_SET = COLOR_WHITE;
//         _delay_loop_1(10);
//     }
// }


// draws 2 lines, as we want a checkmark
void horizonalLineBlank(void){
    horizFontPorch();
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_2(280);
}


// draws half a line, then equalizer
void horizVideoHalf(void){
    // uint8_t n = 5;
    horizFontPorch();
    PORT_SET = COLOR_BLACK;
    _delay_loop_1(112);
    // while(n--){
    //     PORT_SET = COLOR_BLACK;
    //     _delay_loop_1(13);
    //     PORT_SET = COLOR_WHITE;
    //     _delay_loop_1(13);
    // }

    // equalizer with frame
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(12);       // delay for 2.3uS
    PORT_SET = VOLTAGE_BLANKING;
    // _delay_loop_1(157);       // delay for 29.45uS (half of line with 2.3uS out)
}

// draws half a line, then equalizer
void horizVideoHalf2(void){
    // uint8_t n = 5;

    // equalizer with frame
    PORT_SET = VOLTAGE_SYNC;
    _delay_loop_1(12);       // delay for 2.3uS
    PORT_SET = VOLTAGE_BLANKING;
    _delay_loop_1(157);       // delay for 29.45uS (half of line with 2.3uS out)


    horizFontPorch();
    PORT_SET = COLOR_WHITE;
    // _delay_loop_1(112);
    // while(n--){
    //     PORT_SET = COLOR_BLACK;
    //     _delay_loop_1(13);
    //     PORT_SET = COLOR_WHITE;
    //     _delay_loop_1(13);
    // }


}


int main(void){
    // setup PortD to be the main GPIO out
    DDRD = 0xFF;
    // setup Timer0 to fire an interrupt every horizonal line
    TCCR0A = 0x02;      // clear-timer on match mode
    TIMSK0 = 0x02;         // interrupt on match OutputCompareA
    OCR0A = 127;       // for a 63.5uS horizonal pulse
    TCCR0B = 0x02;         // start timer, clock/8

    sei();          // Turn on interrupts

    for(EVER){
    }

    return 0;
}

ISR(TIMER0_COMPA_vect){
    switch(horizN){
        case 1 ... 3:
        case 7 ... 9:
        case 264:
        case 265:
        case 270:
        case 271:
            equalizationLine();
            break;
        case 266:
            equalization();
            _delay_loop_1(157);
            VSync();
            break;
        case 269:
            VSync();
            _delay_loop_1(25);
            equalization();
            lineN = 1;
            break;
        case 10 ... 20:
        case 272 ... 282:
            horizonalLineBlank();
            break;
        case 4:
        case 5:
        case 6:
        case 267:
        case 268:
            VSyncLine();
            break;
        case 263:
            horizVideoHalf();
            break;
        case 525:
            horizN = 0;
            lineN = 0;
            horizonalLine();
            break;
        default:
            horizonalLine();
            lineN += 2;
            break;
    }

    // if(!horizN){
        // _delay_ms(500);
    // }
    horizN++;
}
