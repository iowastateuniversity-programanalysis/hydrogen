#include <stdio.h>
int main() {
  float a, b, area;
  scanf("%f %f", &a, &b);
  area = (a * b) / 2;
  if (area > 0) {
    printf("The area of (%.4f,%.4f),(%.4f,0) and (0,%.4f) is %.4f.", a, b, a, b, area);
  } else {
    printf("The area of (%.4f,%.4f),(%.4f,0) and (0,%.4f) is %.4f.", a, b, a, b, (-1) * area);
  }
  return 0;
}
