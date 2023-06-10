#include "common_porting.h"
//#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "bmp390_task.h"
#include "bmp3.h"

#if defined(USE_BMP390)

extern volatile uint8_t int1_flag;
extern volatile uint8_t int2_flag;

struct bmp3_dev dev;

static uint8_t dev_addr = 0;

void bmp3_check_rslt(const char api_name[], int8_t rslt) {
	switch (rslt) {
	case BMP3_OK:
//		printf("BMP3_OK\r\n");
		/* Do nothing */
		break;
	case BMP3_E_NULL_PTR:
		printf("API [%s] Error [%d] : Null pointer\r\n", api_name, rslt);
		break;
	case BMP3_E_COMM_FAIL:
		printf("API [%s] Error [%d] : Communication failure\r\n", api_name,
				rslt);
		break;
	case BMP3_E_INVALID_LEN:
		printf("API [%s] Error [%d] : Incorrect length parameter\r\n", api_name,
				rslt);
		break;
	case BMP3_E_DEV_NOT_FOUND:
		printf("API [%s] Error [%d] : Device not found\r\n", api_name, rslt);
		break;
	case BMP3_E_CONFIGURATION_ERR:
		printf("API [%s] Error [%d] : Configuration Error\r\n", api_name, rslt);
		break;
	case BMP3_W_SENSOR_NOT_ENABLED:
		printf("API [%s] Error [%d] : Warning when Sensor not enabled\r\n",
				api_name, rslt);
		break;
	case BMP3_W_INVALID_FIFO_REQ_FRAME_CNT:
		printf(
				"API [%s] Error [%d] : Warning when Fifo watermark level is not in limit\r\n",
				api_name, rslt);
		break;
	default:
		printf("API [%s] Error [%d] : Unknown error code\r\n", api_name, rslt);
		break;
//		while(1){};
	}
}

BMP3_INTF_RET_TYPE bmp3_interface_init(struct bmp3_dev *bmp3, uint8_t intf) {
	int8_t rslt = BMP3_OK;

	if (bmp3 != NULL) {
		/* Bus configuration : I2C */
		if (intf == BMP3_I2C_INTF) {
			printf("I2C Interface\n");
			dev_addr = BMP3_ADDR_I2C_SEC;
			bmp3->read = SensorAPI_I2Cx_Read;
			bmp3->write = SensorAPI_I2Cx_Write;
			bmp3->intf = BMP3_I2C_INTF;
		}
		/* Bus configuration : SPI */
		else if (intf == BMP3_SPI_INTF) {
			printf("SPI Interface\n");
			dev_addr = 0;
			bmp3->read = SensorAPI_SPIx_Read;
			bmp3->write = SensorAPI_SPIx_Write;
			bmp3->intf = BMP3_SPI_INTF;
		}

		bmp3->delay_us = bmp3_delay_us;
		bmp3->intf_ptr = &dev_addr;

//		assignment to 'bmp3_write_fptr_t'
//		from incompatible pointer type 'int8_t (*)(uint8_t,  uint8_t *, uint16_t,  void *)'
//
//		{aka 'signed char (*)(unsigned char,  const unsigned char *, long unsigned int,  void *)'}
//		{aka 'signed char (*)(unsigned char,  unsigned char *, short unsigned int,  void *)'}
	} else {
		rslt = BMP3_E_NULL_PTR;
	}

	return rslt;
}

void BMP390_Init(void) {

	int8_t rslt = 0;

	uint8_t settings_sel;
//	struct bmp3_dev dev; // Original, creats a bug
	struct bmp3_settings settings = { 0 };

	/* Interface reference is given as a parameter
	 *		   For I2C : BMP3_I2C_INTF
	 *		   For SPI : BMP3_SPI_INTF
	 */
#if defined(USE_I2C_INTERFACE)
	rslt = bmp3_interface_init(&dev, BMP3_I2C_INTF);
	#elif defined(USE_SPI_INTERFACE)
	rslt = bmp3_interface_init(&dev, BMP3_SPI_INTF);
#endif
	bmp3_check_rslt("bmp3_interface_init", rslt);

	rslt = bmp3_init(&dev);
	bmp3_check_rslt("bmp3_init", rslt);

	settings.int_settings.drdy_en = BMP3_DISABLE;
	settings.press_en = BMP3_ENABLE;
	settings.temp_en = BMP3_ENABLE;

	settings.odr_filter.press_os = BMP3_OVERSAMPLING_8X;
	settings.odr_filter.temp_os = BMP3_OVERSAMPLING_8X;
	settings.odr_filter.odr = BMP3_ODR_12_5_HZ;

	// HV: adding more
	settings.odr_filter.iir_filter = BMP3_IIR_FILTER_COEFF_7; // Enable IIR filter, results will be noisy without this

	settings_sel = BMP3_SEL_PRESS_EN |
	BMP3_SEL_TEMP_EN |
	BMP3_SEL_PRESS_OS |
	BMP3_SEL_TEMP_OS |
	BMP3_SEL_IIR_FILTER | // HV: adding more
//			BMP3_SEL_DRDY_EN | // Data ready interrupt: don't need
			BMP3_SEL_ODR; // |

	rslt = bmp3_set_sensor_settings(settings_sel, &settings, &dev);
	bmp3_check_rslt("bmp3_set_sensor_settings", rslt);

	settings.op_mode = BMP3_MODE_NORMAL;
	rslt = bmp3_set_op_mode(&settings, &dev);
	bmp3_check_rslt("bmp3_set_op_mode", rslt);

}

/*
 * Return pressure in hPa, temperature in deg C
 */
struct bmp3_data bmp390_getdata(void) {
	int8_t rslt = 0;

	// Creating data variable
	struct bmp3_data data = { -1, -1 };
	struct bmp3_status status = { { 0 } };

	rslt = bmp3_get_status(&status, &dev);
	bmp3_check_rslt("bmp3_get_status", rslt);

	/* Read temperature and pressure data iteratively based on data ready interrupt */
	if ((rslt == BMP3_OK) && (status.intr.drdy == BMP3_ENABLE)) {
		/*
		 * First parameter indicates the type of data to be read
		 * BMP3_PRESS_TEMP : To read pressure and temperature data
		 * BMP3_TEMP	   : To read only temperature data
		 * BMP3_PRESS	   : To read only pressure data
		 */
		rslt = bmp3_get_sensor_data(BMP3_PRESS_TEMP, &data, &dev);
		bmp3_check_rslt("bmp3_get_sensor_data", rslt);

		/* NOTE : Read status register again to clear data ready interrupt status */
		rslt = bmp3_get_status(&status, &dev);
		bmp3_check_rslt("bmp3_get_status", rslt);

		data.pressure /= 100.0f; // convert to hPa

		// EXAMPLE
//		printf("Data  T: %.2f deg C, P: %.2f Pa\n", (data.temperature), (data.pressure));
	}

	return data;
}

// Convert from Pa to m
/**************************************************************************/
/*!
 @brief Calculates the altitude (in meters).

 Reads the current atmostpheric pressure (in hPa) from the sensor and
 calculates via the provided sea-level pressure (in hPa).

 @param  seaLevel      Sea-level pressure in hPa
 @return Altitude in meters
 */
/**************************************************************************/
//float Adafruit_BMP3XX::readAltitude(float seaLevel) {
// Equation taken from BMP180 datasheet (page 16):
//  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
// Note that using the equation from wikipedia can give bad results
// at high altitude. See this thread for more information:
//  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
float elevation_conversion(double atmospheric) {
//	float atmospheric = pressure / 100.0f;
	float elevation = 44330.0
			* (1.0 - pow(atmospheric / SEA_LEVEL_PRESSURE_HPA, 0.1903));
	if (isnan(elevation) || atmospheric == -1) {
		return -1;
	}
	else {
		return elevation;
	}
}

#endif
