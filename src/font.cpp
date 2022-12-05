/**
 * font.cpp contains functions for printing characters to an array, using a font defined in font.h
 */
#include "font.h"
/**
 * @brief  prints a character to a given image array using a 5x8 font
 * @param  c: byte (0-255) value representing character to display. In addition to the standard ASCII values for letters, font.h defines symbols for all other values.
 * @param  x_pos: what x coordinate (column) of the image array should the first column of the character be printed into? if >=width, it is wrapped around to the start of the array inside this function.
 * @param  foreground: CRGB or CHSV (FastLED) color for the foreground of the character
 * @param  background: CRGB or CHSV (FastLED) color for the background of the character
 * @param  image[][8]: array of CRGB for the character to be printed into
 * @param  width: first dimension of the image array.
 * @retval void
 */
void printChar(byte c, long x_pos, CRGB foreground, CRGB background, CRGB image[][8], int width)
{
    for (int x = 0; x < 6; x++) { // assumes characters are 5 wide
        int column = (((x_pos % width) + width + x) % width); // wraps around to within [0,width) even if x_pos is negative
        for (int y = 0; y < 8; y++) { // assumes screen is 8 characters tall
            if (x < 5)
                image[column][y] = bitRead(font[c * 5 + x], 7 - y) ? foreground : background;
            else // spacing between charaters
                image[column][y] = background;
        }
    }
}

/**
 * @brief  prints a string of characters to an array of pixels
 * @note  prints characters from left to right, because of how printChar() works, individual characters will be split when wrapping from the end to the beginning of the image array
 * @param  str: char* (null terminated string) to print
 * @param  x_pos: what x coordinate (column) of the image array should the first column of the first character be printed into? if >=width, it is wrapped around to the start of the array inside this function.
 * @param  foreground: CRGB or CHSV (FastLED) color for the foreground of the character
 * @param  background: CRGB or CHSV (FastLED) color for the background of the character
 * @param  image[][8]: array of CRGB for the character to be printed into
 * @param  width: first dimension of the image array.
 * @retval void
 */
void printString(char* str, long x_pos, CRGB foreground, CRGB background, CRGB image[][8], int width)
{
    for (unsigned int i = 0; i <= strlen(str); i++) {
        printChar((byte)str[i], x_pos + i * 6 /*char spacing*/, foreground, background, image, width);
    }
}
