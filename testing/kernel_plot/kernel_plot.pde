final float KERNEL_SUPPORT_H = 2.0;

final float KERNEL_ALPHA = 1 / (4 * PI * KERNEL_SUPPORT_H * KERNEL_SUPPORT_H * KERNEL_SUPPORT_H);

float kernel(PVector xi, PVector xj) {
  PVector d = PVector.sub(xi, xj).div(KERNEL_SUPPORT_H);
  float q = d.mag();
  float t1 = max(1 - q, 0.0f);
  float t2 = max(2 - q, 0.0f);
  return KERNEL_ALPHA * (t2 * t2 * t2 - 4 * t1 * t1 * t1);
}

PVector kernel_deriv_rev(PVector xi, PVector xj) {
  PVector d = PVector.sub(xi, xj).div(KERNEL_SUPPORT_H);
  float q = d.mag();
  float t1 = max(1 - q, 0.0f);
  float t2 = max(2 - q, 0.0f);
  return PVector.mult(d, KERNEL_ALPHA / q * (-3 * t2 * t2 + 12 * t1 * t1));
}

void setup() {
  size(1000, 1000);
  scale(1, -1);
  translate(0, -height/2);
  for (int i = 0; i < width; i++) {
    float x = 4.0*(2.0*float(i)/float(width)-1.0);
    point(i, 10*height* kernel_deriv_rev(new PVector(0, 0, 0), new PVector(x, 0, 0)).x);
    point(i, 10*height* kernel(new PVector(0, 0, 0), new PVector(x, 0, 0)));
  }
}
