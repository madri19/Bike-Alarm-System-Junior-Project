#ifndef _ACCEL_LIB_H
#define _ACCEL_LIB_H

void i2c_wait(unsigned int cnt);
void enableI2C();
void setupAccerlerometerI2C();
void clearAccelerometerInterruptFlag();

#endif
