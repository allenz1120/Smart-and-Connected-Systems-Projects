// John Kircher, Alex Prior, Allen Zou
// 10/8/2020

/*
  - Barebones example using one timer as an interval alarm
  - Code prints every 1 second
  - Adapted and simplified from ESP-IDF timer example
  - See examples for more timer functionality

  Emily Lam, September 2019

*/

//Code chunks that we edited starts around line 367

#include <stdio.h>
#include <math.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "esp_attr.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "./ADXL343.h"

#define TIMER_DIVIDER 16                             //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // to seconds
#define TIMER_INTERVAL_SEC (1)                       // Sample test interval for the first timer
#define TEST_WITH_RELOAD 1                           // Testing will be done with auto reload
#define GPIOFLAG 14

#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value

// ADXL343
#define SLAVE_ADDR                         ADXL343_ADDRESS // 0x53

// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init(){
  // Debug
  //printf("\n>> i2c Config\n");
  int err;

  // Port configuration
  int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

  /// Define I2C configurations
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;                              // Master mode
  conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
  err = i2c_param_config(i2c_master_port, &conf);           // Configure
  if (err == ESP_OK) {
      //printf("- parameters: ok\n");
      }

  // Install I2C driver
  err = i2c_driver_install(i2c_master_port, conf.mode,
                     I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                     I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
  if (err == ESP_OK) {
      //printf("- initialized: yes\n");
      }

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return err;
}

// Utility function to scan for i2c device
static void i2c_scanner() {
  int32_t scanTimeout = 1000;
  //printf("\n>> I2C scanning ..."  "\n");
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++) {
    // printf("0x%X%s",i,"\n");
    if (testConnection(i, scanTimeout) == ESP_OK) {
      //printf( "- Device found at address: 0x%X%s", i, "\n");
      count++;
    }
  }
  if (count == 0) {
      //printf("- No I2C devices found!" "\n");
      }
}

////////////////////////////////////////////////////////////////////////////////

// ADXL343 Functions ///////////////////////////////////////////////////////////

// Get Device ID
int getDeviceID(uint8_t *data) {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

// Write one byte to register
void writeRegister(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //slave address followed by write bit
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    //register pointer sent
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //data sent
    i2c_master_write_byte(cmd, data, ACK_CHECK_DIS);
    //stop command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

// Read register
uint8_t readRegister(uint8_t reg) {
    uint8_t value;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //slave followed by write bit
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    //register pointer sent
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //repeated start command
    i2c_master_start(cmd);
    //slave followed by read bit
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
    //place data from register into bus
    i2c_master_read_byte(cmd, &value, ACK_CHECK_DIS);
    //stop command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return value;
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
    //Combining the two values into a signed-word (16 bit)
    //by ORing the LSB with the MSB shifted to the left by 8
    uint8_t val1;
    uint8_t val2;
    val1 = readRegister(reg);
    if (reg == 41) {
        val2 = 0;
    } else {
        val2 = readRegister(reg+1);
    }
    //bit shift operation, from stack overflow on bitwise operators and converting.
    return (((int16_t)val2 << 8) | val1);
}

void setRange(range_t range) {
  // Red the data format register to preserve bits
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);

  // Update the data rate
  format &= ~0x0F;
  format |= range;

  // Make sure that the FULL-RES bit is enabled for range scaling
  format |= 0x08;

  // Write the register back to the IC
  writeRegister(ADXL343_REG_DATA_FORMAT, format);

}

range_t getRange(void) {
  // Red the data format register to preserve bits
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}

dataRate_t getDataRate(void) {
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}

////////////////////////////////////////////////////////////////////////////////

// function to get acceleration
void getAccel(float * xp, float *yp, float *zp) {
  *xp = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *yp = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *zp = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
 printf("X,%.2f\n", *xp);
 printf("Y,%.2f\n", *yp);
 printf("Z,%.2f\n", *zp);


}

// function to print roll and pitch
void calcRP(float x, float y, float z){
  float roll = atan2(y , z) * 57.3;
  float pitch = atan2((-1*x) , sqrt(y*y + z*z)) * 57.3;
  printf("Roll,%.2f\n", roll);
  printf("Pitch,%.2f\n", pitch);

}

// Task to continuously poll acceleration and calculate roll and pitch
static void accelHandler() {
  //printf("\n>> Polling ADAXL343\n");
  while (1) {
    float xVal, yVal, zVal;
    getAccel(&xVal, &yVal, &zVal);
    calcRP(xVal, yVal, zVal);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

}


// ADC initialization
static esp_adc_cal_characteristics_t *adc_chars;

// Thermistor initialization
#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling
int counter = 1;
int voltageRunSum = 0;
int voltageAVG = 0;
char tempStr[5];

//Thermistor ADC pin
static const adc_channel_t thermistorChannel = ADC_CHANNEL_6; //GPIO34 if ADC1


static const adc_atten_t atten = ADC_ATTEN_DB_11; // Changed attenuation to extend range of measurement up to approx. 2600mV (Appears to accurately read input voltage up to at least 3300mV though)
static const adc_unit_t unit = ADC_UNIT_1;

static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        // printf("eFuse Two Point: Supported\n");
    }
    else
    {
        // printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        // printf("eFuse Vref: Supported\n");
    }
    else
    {
        // printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        // printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        // printf("Characterized using eFuse Vref\n");
    }
    else
    {
        // printf("Characterized using Default Vref\n");
    }
}



//This task is responsible for reading the thermistor data
static void thermoHandler()
{
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(thermistorChannel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)thermistorChannel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)thermistorChannel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)thermistorChannel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;

        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        voltageRunSum += voltage;
        // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        // printf("Resistance: %f\n", resistance);

        if (counter == 2)
        {
            counter = 0;
            float resistance = (10000 * (3.3 - voltage / 1000.0)) / (voltage / 1000.0);
            float temperature = 1.0 / ((1.0 / 298) + (1.0 / 3435.0) * log(resistance / 10000.0)); // Simplified B-parameter Steinhart-Hart equation
            temperature -= 273.15;

            printf("Temperature,%f\n", temperature);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        counter++;
    }
}




void app_main(void)
{
    // Create a FIFO queue for timer-based
    // Routine
    i2c_master_init();
    i2c_scanner();

    // Check for ADXL343
    uint8_t deviceID;
    getDeviceID(&deviceID);
    if (deviceID == 0xE5) {
        //printf("\n>> Found ADAXL343\n");
    }

    // Disable interrupts
    writeRegister(ADXL343_REG_INT_ENABLE, 0);

    // Set range
    setRange(ADXL343_RANGE_16_G);

    // Enable measurements
    writeRegister(ADXL343_REG_POWER_CTL, 0x08);

    // Create task to poll ADXL343
    xTaskCreate(thermoHandler, "thermoHandler_task", 4096, NULL, 6, NULL);
    xTaskCreate(accelHandler,"accelHandler_task", 4096, NULL, 5, NULL);

    //xTaskCreate(ultrasonicHandler, "ultrasonicHandler_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    //xTaskCreate(IRhandler, "IRhandler_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
}
