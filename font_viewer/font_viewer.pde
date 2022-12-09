// https://processing.org/ program (Java) for testing font array decoding

PGraphics pg;
color image[][]=new color[6][8];
void setup() {
  size(600, 800);
  noSmooth();
  textSize(30);
  fill(0, 255, 0);
  pg=createGraphics(6, 8);
  pg.noSmooth();
}
void draw() {
  pg.beginDraw();
  pg.background(0);
  printChar((mouseX*255/width));
  for (int column=0; column<6; column++) {
    for (int row=0; row<8; row++) {
      pg.set(column, 7-row, image[column][row]);
    }
  }
  pg.endDraw();
  image(pg, 0, 0, 600, 800);
}
void printChar(int c) {
  for (int x = 0; x < 6; x++) { // assumes characters are 5 wide plus one column of space
    for (int y = 0; y < 8; y++) { // assumes screen is 8 characters tall
      if (x < 5) {
        image[x][y] = bitRead(font[c * 5 + x], 7 - y) ? color(255) : color(0);
      } else { // spacing between characters
        image[x][y] = color(0);
      }
    }
  }
}

public boolean bitRead(int ID, int position)
{
  return ((ID >> position) & 1)==1;
}
