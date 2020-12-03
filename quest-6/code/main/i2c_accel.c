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

#include <ctype.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "sdkconfig.h"

// GLOBAL VARIABLES
int range;         // Global variable for ultrasonic range
int lidarDistance; // Global I2C Lidar Variable

#define TIMER_DIVIDER 16                             //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // to seconds
#define TIMER_INTERVAL_SEC (1)                       // Sample test interval for the first timer
#define TEST_WITH_RELOAD 1                           // Testing will be done with auto reload
#define GPIOFLAG 14
#define BLINK_GPIO 13

#define I2C_EXAMPLE_MASTER_SCL_IO 22        // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO 23        // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM I2C_NUM_0    // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ 100000   // i2c master clock freq
#define WRITE_BIT I2C_MASTER_WRITE          // i2c master write
#define READ_BIT I2C_MASTER_READ            // i2c master read
#define ACK_CHECK_EN true                   // i2c master will check ack
#define ACK_CHECK_DIS false                 // i2c master will not check ack
#define ACK_VAL 0x00                        // i2c ack value
#define NACK_VAL 0xFF                       // i2c nack value

#define SLAVE_ADDR 0x62 // Lidar slave address

#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180      //Maximum angle in degree upto which servo can rotate

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 26); //Set GPIO 26 as PWM0A DOG
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 25); //Set GPIO 25 as PWM0B CAT
}

// static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
// {
//     uint32_t cal_pulsewidth = 0;
//     cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
//     return cal_pulsewidth;
// }

void pwm_init()
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50; //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings
}

/* BSD Socket API Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "freertos/event_groups.h"
// #include "addr_from_stdin.h"

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR "192.168.1.9"
#endif

#define PORT 9001

static const char *TAG = "example";
static const char *payload = "";

#define MAX 100
char payTemp[MAX] = "24.312342";
char payX[MAX];
char payY[MAX];
char payZ[MAX];
char payP[MAX];
char payR[MAX];
char fullPay[MAX];
// static const char nkMessage[128]="Nk!";
// static const char okMessage[128]="Ok!";

// static void udp_client_task(void *pvParameters)
// {
//     char rx_buffer[128];
//     char host_ip[] = HOST_IP_ADDR;
//     int addr_family = 0;
//     int ip_protocol = 0;
//     gpio_reset_pin(BLINK_GPIO);
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
//     int ledOn = 0;

//     while (1)
//     {

//         // #if defined(CONFIG_EXAMPLE_IPV4)
//         struct sockaddr_in dest_addr;
//         dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;
//         // #elif defined(CONFIG_EXAMPLE_IPV6)
//         //         struct sockaddr_in6 dest_addr = { 0 };
//         //         inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
//         //         dest_addr.sin6_family = AF_INET6;
//         //         dest_addr.sin6_port = htons(PORT);
//         //         dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
//         //         addr_family = AF_INET6;
//         //         ip_protocol = IPPROTO_IPV6;
//         // #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
//         //         struct sockaddr_in6 dest_addr = { 0 };
//         //         ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
//         // #endif

//         int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
//         if (sock < 0)
//         {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

//         while (1)
//         {
//             // fullPay=*payX+","+*payY+","+*payZ+","+*payR+","+*payP+","+*payTemp;
//             strcat(payX, ",");
//             strcat(payX, payY);
//             strcat(payX, ",");
//             strcat(payX, payZ);
//             strcat(payX, ",");
//             strcat(payX, payR);
//             strcat(payX, ",");
//             strcat(payX, payP);
//             strcat(payX, ",");
//             strcat(payX, payTemp);
//             // printf("--------------FULL PAY IS: %s \n",payX);
//             payload = payX;
//             // printf("--------------PAYLOAD IS: %s \n",payload);
//             int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//             if (err < 0)
//             {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;
//             }
//             ESP_LOGI(TAG, "Message sent");

//             struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
//             socklen_t socklen = sizeof(source_addr);
//             int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

//             // Error occurred during receiving
//             if (len < 0)
//             {
//                 ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
//                 break;
//             }
//             // Data received
//             else
//             {
//                 rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
//                 if (rx_buffer == 0)
//                 {
//                     break;
//                 }
//                 ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
//                 ESP_LOGI(TAG, "%s", rx_buffer);
//                 printf(rx_buffer);
//                 printf("-----------------");
//                 if (strncmp(rx_buffer, "Ok!", 3) == 0)
//                 {
//                     // ledOn = 1;
//                     printf("TOGGLED--------------");
//                     ESP_LOGI(TAG, "Toggle LED");
//                     if (ledOn == 0)
//                     {
//                         gpio_set_level(BLINK_GPIO, 1);
//                         ledOn = 1;
//                     }
//                     else if (ledOn == 1)
//                     {
//                         gpio_set_level(BLINK_GPIO, 0);
//                         ledOn = 0;
//                     }
//                     //break;
//                 }
//                 else if (strncmp(rx_buffer, "Nk!", 3) == 0)
//                 {
//                     printf("STAYING--------------");
//                     ESP_LOGI(TAG, "Hold LED");

//                     //break;
//                 }
//             }

//             vTaskDelay(2000 / portTICK_PERIOD_MS);
//         }

//         if (sock != -1)
//         {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }
//     }
//     vTaskDelete(NULL);
// }

// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init()
{
    // Debug
    //printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                        // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;        // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;        // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ; // CLK frequency
    err = i2c_param_config(i2c_master_port, &conf);     // Configure
    if (err == ESP_OK)
    {
        //printf("- parameters: ok\n");
    }

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK)
    {
        //printf("- initialized: yes\n");
    }

    // Data in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Utility function to scan for i2c device
static void i2c_scanner()
{
    int32_t scanTimeout = 1000;
    //printf("\n>> I2C scanning ..."  "\n");
    uint8_t count = 0;
    for (uint8_t i = 1; i < 127; i++)
    {
        // printf("0x%X%s",i,"\n");
        if (testConnection(i, scanTimeout) == ESP_OK)
        {
            //printf( "- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }
    if (count == 0)
    {
        //printf("- No I2C devices found!" "\n");
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
static const adc_channel_t ultrasonicChannel = ADC_CHANNEL_3; // GPIO39
static const adc_channel_t thermistorChannel = ADC_CHANNEL_6; // GPIO34 if ADC1

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

            sprintf(payTemp, "%f", temperature);

            printf("Temperature,%f\n", temperature);
            //strcat((char*)&temperature, payload);
            // printf("Payload: %s\n", payload);
        }
        vTaskDelay(100);
        counter++;
    }
}

void ultrasonicData(void *arg)
{
    //Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)ultrasonicChannel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)ultrasonicChannel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        uint32_t vcm = 3.222;  //conversion to get volts per centimeter. This is found by 3.3V / 1024
        range = voltage / vcm; //calculation to get range in centimeters.
        printf("Ultrasonic Distance: %dcm\n", range);
        vTaskDelay(100); //random number rn CHANGE!!
    }
}

// void catServo(void *arg)
// {
//     uint32_t catAngle;
//     uint32_t defaultAngle;

//     catAngle = servo_per_degree_init(0);
//     defaultAngle = servo_per_degree_init(90);

//     if (flag variable toggled from button press)
//     {
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, catAngle);
//         vTaskDelay(100);
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, defaultAngle);
//         vTaskDelay(100);
//     }
// }

// void dogServo(void *arg)
// {
//     uint32_t dogAngle;
//     uint32_t defaultAngle;

//     catAngle = servo_per_degree_init(0);
//     defaultAngle = servo_per_degree_init(90);

//     if (flag variable toggled from button press)
//     {
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, dogAngle);
//         vTaskDelay(100);
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, defaultAngle);
//         vTaskDelay(100);
//     }
// }

// Write one byte to register
void writeRegister(uint8_t reg, uint8_t data)
{
    // int ret;
    // printf("--Writing %d to reg %d!--\n", data, reg);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //slave address followed by write bit
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    //register pointer sent
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //data sent
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    //stop command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    // if (ret == ESP_OK)
    // {
    //   printf("I2C SUCCESSFUL \n");
    // }
    i2c_cmd_link_delete(cmd);
}

// Read register
uint8_t readRegister(uint8_t reg)
{
    uint8_t value;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //start command
    i2c_master_start(cmd);
    //slave followed by write bit
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    //register pointer sent
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //stop command
    // i2c_master_stop(cmd);

    //repeated start command
    i2c_master_start(cmd);
    //slave followed by read bit
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    //place data from register into bus
    i2c_master_read_byte(cmd, &value, ACK_CHECK_DIS);
    //stop command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return value;
}

static void lidarRead()
{
    uint8_t initReg = 0x00;
    uint8_t initData = 0x04;
    uint8_t data = 0x06;
    vTaskDelay(22);
    while (1)
    {
        // Initialize lidar device

        writeRegister(initReg, initData);
        data = readRegister(0x01);
        while ((data & 1) != 0x00)
        {
            data = readRegister(0x01);
            // printf("content of reg 1 LSB is %x \n", (data & 1));
            vTaskDelay(10);
        }
        // printf("content of reg 1 LSB outsie of the while loop is %x \n", (data & 1));
        // if (data == 0)
        // {
        // Read high and low distance bits off device
        uint8_t distHigh = readRegister(0x11);
        uint8_t distLow = readRegister(0x10);

        // // Print high and low distance data
        // printf("distHigh is: %x \n", distHigh);
        // printf("distLow is: %x \n", distLow);

        //Add the two distances into a 16 bit int
        lidarDistance = (distHigh << 8) + distLow;

        printf("Lidar Distance is: %d \n", lidarDistance);
        // }
        vTaskDelay(100);
    }
}

void app_main(void)
{
    // Create a FIFO queue for timer-based
    // Routine
    pwm_init();
    i2c_master_init();
    i2c_scanner();

    xTaskCreate(thermoHandler, "thermoHandler_task", 4096, NULL, 6, NULL);
    xTaskCreate(ultrasonicData, "ultrasonicData", 4096, NULL, 5, NULL);
    xTaskCreate(lidarRead, "lidarRead", 4096, NULL, 5, NULL);
    // xTaskCreate(catServo, "catServo", 4096, NULL, 4, NULL);
    // xTaskCreate(dogServo, "dogServo", 4096, NULL, 4, NULL);

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // ESP_ERROR_CHECK(example_connect());

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}
