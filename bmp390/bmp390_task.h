#ifndef BMP390_H__
#define BMP390_H__

#include <stdint.h>
#include "user_define.h"
#include "bmp3_defs.h"
#include "math.h"


#if defined(USE_BMP390)


void BMP390_Init(void);
struct bmp3_data BMP390_read(void);
float elevation_conversion(double pressure);

#endif

#endif

