#ifndef BMP390_H__
#define BMP390_H__

#include <stdint.h>
#include "user_define.h"
#include "bmp3_defs.h"
#include "math.h"


#if defined(USE_BMP390)


void BMP390_Init(void);
struct bmp3_data bmp390_getdata(void);
float convert_Pa_to_meter(double pressure_Pa);
float convert_mhPa_to_meter(int32_t pressure_mhPa);
int32_t convert_Pa_to_mhPa(double pressure_Pa);
struct bmp3_data bmp390_data_check(struct bmp3_data data);

#endif

#endif

