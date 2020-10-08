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

#define TIMER_DIVIDER 16                             //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // to seconds
#define TIMER_INTERVAL_SEC (1)                       // Sample test interval for the first timer
#define TEST_WITH_RELOAD 1                           // Testing will be done with auto reload
#define GPIOFLAG 14

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
// IR ADC pin
static const adc_channel_t irChannel = ADC_CHANNEL_4; //GPIO32 if ADC1
// Ultrasonic ADC pin
static const adc_channel_t ultrasonicChannel = ADC_CHANNEL_5; //GPIO33 if ADC1

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

// // A simple structure to pass "events" to main task
// typedef struct
// {
//     int flag; // flag for enabling stuff in main code
// } timer_event_t;

// // Initialize queue handler for timer-based events
// xQueueHandle timer_queue;

// // ISR handler
// void IRAM_ATTR timer_group0_isr(void *para)
// {

//     // Prepare basic event data, aka set flag
//     timer_event_t evt;
//     evt.flag = 1;

//     // Clear the interrupt, Timer 0 in group 0
//     TIMERG0.int_clr_timers.t0 = 1;

//     // After the alarm triggers, we need to re-enable it to trigger it next time
//     TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

//     // Send the event data back to the main program task
//     xQueueSendFromISR(timer_queue, &evt, NULL);
// }

// // Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
// static void init()
// {
//     const uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = UART_SCLK_APB,
//     };
//     // We won't use a buffer for sending data.
//     uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
//     uart_param_config(UART_NUM_1, &uart_config);
//     // uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//     gpio_set_direction(GPIOFLAG, GPIO_MODE_INPUT);
//     /* Select and initialize basic parameters of the timer */
//     timer_config_t config;
//     config.divider = TIMER_DIVIDER;
//     config.counter_dir = TIMER_COUNT_UP;
//     config.counter_en = TIMER_PAUSE;
//     config.alarm_en = TIMER_ALARM_EN;
//     config.intr_type = TIMER_INTR_LEVEL;
//     config.auto_reload = TEST_WITH_RELOAD;
//     timer_init(TIMER_GROUP_0, TIMER_0, &config);

//     // Timer's counter will initially start from value below
//     timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

//     // Configure the alarm value and the interrupt on alarm
//     timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_SEC * TIMER_SCALE);
//     timer_enable_intr(TIMER_GROUP_0, TIMER_0);
//     timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
//                        (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

//     // Start timer
//     timer_start(TIMER_GROUP_0, TIMER_0);

//     int err;
// }

// // The main task of this example program
// static void timer_evt_task(void *arg)
// {
//     while (1)
//     {
//         // Create dummy structure to store structure from queue
//         timer_event_t evt;

//         // Transfer from queue
//         xQueueReceive(timer_queue, &evt, portMAX_DELAY);

//         // Decremenet counter if triggered
//         if (evt.flag == 1)
//         {
//             counter--;
//             printf("%d\n", counter);
//         }
//     }
//     vTaskDelete(NULL);
// }

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
            // snprintf(tempStr, sizeof(tempStr), "%.2f", temperature);
            // const int temp = uart_write_bytes(UART_NUM_1, tempStr, sizeof(tempStr));

            // voltageAVG = voltageRunSum / 10;
            // voltageRunSum = 0;

            // int voltage1 = (voltageAVG / 1000) % 10;
            // int voltage2 = (voltageAVG / 100) % 10;
            // int voltage3 = (voltageAVG / 10) % 10;
            // int voltage4 = voltageAVG % 10;
            // printf("%d %d %d %d\n", voltage1, voltage2, voltage3, voltage4);

            // Write to characters to buffer
            // displaybuffer[0] = alphafonttable[voltage1];
            // displaybuffer[1] = alphafonttable[voltage2];
            // displaybuffer[2] = alphafonttable[voltage3];
            // displaybuffer[3] = alphafonttable[voltage4];
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        counter++;
    }
}

// This task is responsible for reading the IR sensor data
static void IRhandler()
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(irChannel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)irChannel, atten);
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
                adc_reading += adc1_get_raw((adc1_channel_t)irChannel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)irChannel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        uint32_t range = 146060 * (pow(voltage, -1.126));
        printf("irDistance,%d\n", range);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

//This task is responsible for reading the ultrasonic data
static void ultrasonicHandler()
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(ultrasonicChannel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)ultrasonicChannel, atten);
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
        uint32_t vcm = 3.222;           //conversion to get volts per centimeter. This is found by 3.3V / 1024
        uint32_t range = voltage / vcm; //calculation to get range in centimeters.
        printf("ultraDistance,%d\n", range);
        vTaskDelay(pdMS_TO_TICKS(1000)); //2 second delay
    }
}

void app_main(void)
{
    // Create a FIFO queue for timer-based
    // timer_queue = xQueueCreate(10, sizeof(timer_event_t));

    // Initiate alarm using timer API
    // init();
    // Create task to handle timer-based events
    // xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
    xTaskCreate(thermoHandler, "thermoHandler_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(ultrasonicHandler, "ultrasonicHandler_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(IRhandler, "IRhandler_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
}
