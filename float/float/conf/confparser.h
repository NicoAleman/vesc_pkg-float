// This file is autogenerated by VESC Tool

#ifndef CONFPARSER_H_
#define CONFPARSER_H_

#include "datatypes.h"
#include <stdint.h>
#include <stdbool.h>

// Constants
#define FLOAT_CONFIG_SIGNATURE		2736490803

// Functions
int32_t confparser_serialize_float_config(uint8_t *buffer, const float_config *conf);
bool confparser_deserialize_float_config(const uint8_t *buffer, float_config *conf);
void confparser_set_defaults_float_config(float_config *conf);

// CONFPARSER_H_
#endif
