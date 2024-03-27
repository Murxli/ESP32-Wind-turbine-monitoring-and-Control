// vibration_struct.h
#ifndef VIBRATION_STRUCT_H
#define VIBRATION_STRUCT_H

struct Vibration {
  float acceleration[3];
  float rotation[3];
  float internalTemperature;
};

#endif // VIBRATION_STRUCT_H
 