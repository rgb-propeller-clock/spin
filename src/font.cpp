#include "font.h"
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

void printString(char* str, long x_pos, CRGB foreground, CRGB background, CRGB image[][8], int width)
{
    for (unsigned int i = 0; i <= strlen(str); i++) {
        printChar((byte)str[i], x_pos + i * 6 /*char spacing*/, foreground, background, image, width);
    }
}
