//John Kircher, 11/16/2020

/* servo motor control example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// ADC Includes
#include "driver/adc.h"
#include "esp_adc_cal.h"

// ADC Defines
#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling

// Global ADC variable
int range;
int counter;

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC1_CHANNEL_3;     //GPIO39 if ADC1, GPIO14 if ADC2
static const adc_channel_t speedChannel = ADC_CHANNEL_6; //GPIO39 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define DRIVE_MIN_PULSEWIDTH 900     //Minimum pulse width in microsecond
#define DRIVE_MAX_PULSEWIDTH 1900    //Maximum pulse width in microsecond
#define DRIVE_MAX_DEGREE 180         //Maximum angle in degree upto which servo can rotate
#define STEERING_MIN_PULSEWIDTH 700  //Minimum pulse width in microsecond
#define STEERING_MAX_PULSEWIDTH 2100 //Maximum pulse width in microsecond
#define STEERING_MAX_DEGREE 180      //Maximum angle in degree upto which servo can rotate

// PID -----------------------------------------

// Flag for dt

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 26); //Set GPIO 26 as PWM0A, to which drive wheels are connected
    //mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 25); //Set GPIO 25 as PWM0A, to which steering servo is connected
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t drive_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (DRIVE_MIN_PULSEWIDTH + (((DRIVE_MAX_PULSEWIDTH - DRIVE_MIN_PULSEWIDTH) * (degree_of_rotation)) / (DRIVE_MAX_DEGREE)));
    return cal_pulsewidth;
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t steering_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (STEERING_MIN_PULSEWIDTH + (((STEERING_MAX_PULSEWIDTH - STEERING_MIN_PULSEWIDTH) * (degree_of_rotation)) / (STEERING_MAX_DEGREE)));
    return cal_pulsewidth;
}

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

void calibrateESC()
{
    vTaskDelay(3000 / portTICK_PERIOD_MS);                                // Give yourself time to turn on crawler
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 2100); // HIGH signal in microseconds - backwards
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 700); // LOW signal in microseconds - forwards
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1400); // NEUTRAL signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1400); // reset the ESC to neutral (non-moving) value
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

/**
 * @brief Configure MCPWM module
 */
// void drive_control(void *arg)
// {
//     uint32_t angle, count;

//     for (count = 1400; count > 1200; count -= 5)
//     {
//         printf("DRIVING--------------------");
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, count);
//         vTaskDelay(100 / portTICK_RATE_MS);
//     }

//     for (count = 1200; count < 1600; count += 5)
//     {
//         printf("DRIVING--------------------");
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, count);
//         vTaskDelay(100 / portTICK_RATE_MS);
//     }

//     for (count = 1600; count >= 1400; count -= 5)
//     {
//         printf("DRIVING--------------------");
//         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, count);
//         vTaskDelay(100 / portTICK_RATE_MS);
//     }

//     mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1400);
//     vTaskDelay(100 / portTICK_RATE_MS);

//     //}
//     //}
//     vTaskDelete(NULL);
// }

/**
 * @brief Configure MCPWM module
 */

int dt_complete = 0;

int dt = 100;
int setpoint = 100;

float previous_error = 0.00; // Set up PID loop
float integral = 0.00;
float derivative;
float output;
int error;

int Kp;
int Kd;
int Ki;

void PID()
{
    integral = 0.00;
    previous_error = 0.00;
    while (1)
    {
        error = setpoint - range;
        printf("\nError is: %d\n", error);
        if (error > 49)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1400);
            printf("Range: %d \n", range);
            printf("1400 \n");
        }
        else if (error > 10)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1305);
            printf("Range: %d \n", range);
            printf("1300 \n");
        }
        else if (error <= 5)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1285);
            printf("Range: %d \n", range);
            printf("1280 \n");
        }

        integral = integral + error * dt;
        derivative = (error - previous_error) / dt;
        output = Kp * error + Ki * integral + Kd * derivative;
        previous_error = error;
        vTaskDelay(dt);
    }
}

static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        printf("Characterized using eFuse Vref\n");
    }
    else
    {
        printf("Characterized using Default Vref\n");
    }
}

void steering_control(void *arg)
{
    uint32_t right, center, left;

    while (1)
    {

        //count = 90;

        right = steering_per_degree_init(0);
        slightyMoreRight = steer_per_degree_init(35);
        slightlyRight = sttering_per_degree_init(70);

        center = steering_per_degree_init(105);

        left = steering_per_degree_init(180);
        slightlyMoreLeft = steer_per_degree_init(150);
        slightlyLeft = steer_per_degree_init(120);

        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, right);
        vTaskDelay(1000 / portTICK_RATE_MS);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, center);
        vTaskDelay(1000 / portTICK_RATE_MS);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, left);
        vTaskDelay(1000 / portTICK_RATE_MS);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, center);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void speedCalc(void *arg)
{
    while (1)
    {
        counter = 0;
        vTaskDelay(100);
        printf("Counter: %d \n", counter);
        float wheelSpeed = (counter * (2 * 3.14159 * 7 / 12)) / 100;
        printf("Wheel Speed: %.1f m/s \n", wheelSpeed);
        counter = 0;
    }
}

void opticalData(void *arg)
{
    counter = 0;

    bool pulsed = false;
    //Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)speedChannel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)speedChannel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;

        if (adc_reading < 4095 && pulsed == false)
        {
            counter++;
            printf(" ");
            // printf("Counter incremented to: %d \n", counter);
            pulsed = true;
        }
        else if (adc_reading == 4095)
        {
            pulsed = false;
        }
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
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        uint32_t vcm = 3.222;  //conversion to get volts per centimeter. This is found by 3.3V / 1024
        range = voltage / vcm; //calculation to get range in centimeters.
        printf("Raw: %d\tCentimeters: %dcm\n", adc_reading, range);
        vTaskDelay(pdMS_TO_TICKS(1000)); //1 second delay
    }
}

void app_main(void)
{
    pwm_init();
    calibrateESC();

    printf("Testing servo motor.......\n");
    // xTaskCreate(drive_control, "drive_control", 4096, NULL, 4, NULL);
    //xTaskCreate(steering_control, "steering_control", 4096, NULL, 5, NULL);

    // PID Task
    xTaskCreate(PID, "PID", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    xTaskCreate(ultrasonicData, "ultrasonicData", 4096, NULL, 5, NULL);
    xTaskCreate(speedCalc, "speedCalc", 4096, NULL, 5, NULL);
    xTaskCreate(opticalData, "opticalData", 4096, NULL, 5, NULL);
}