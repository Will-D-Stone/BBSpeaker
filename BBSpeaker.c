/*BBSpeaker is code for a beaglebone to initialize two push button switches with each one emitting a different frequency on a speaker
W. Stone original code written for LAB3 Umass ECE231 Spring 2024*/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


// setup input pin in given mode
void config_pin(char* pin_number, char* mode){
    // creates environment to execute shell command
    if(!vfork()){
        // execute shell command for pin configuration
        int ret = execl("/usr/bin/config-pin", "config-pin", pin_number, mode, NULL);
        if (ret < 0){
            printf("Failed to configure pin in PWM mode.\n");
            exit(-1);
        }
    }
}

void set_pwm_duty_cycle(char* pwmchip, char* channel, char* duty_cycle){
    // export file path
    char PWMDutyCycle[60];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-4:%s/duty_cycle", pwmchip,
    channel);
    // configure PWM device
    FILE* fp = fopen(PWMDutyCycle, "w");
    fwrite(duty_cycle, sizeof(char), strlen(duty_cycle), fp);
    fclose(fp);
}

void set_pwm_period(char* pwmchip, char* channel, char* period){
    long duty_cycle_int, period_int;
    // before setting up the period read old duty cycle
    char PWMDutyCycle[60], duty_cycle_str[20];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-4:%s/duty_cycle", pwmchip, channel);
    FILE* fp = fopen(PWMDutyCycle, "r");
    fscanf(fp, "%ld", &duty_cycle_int);
    fclose(fp);
    period_int = atol(period);
    // If the old duty_cycle value is greater than the new period
    // update the dummy_duty_cycle first to avoid errors with setting up
    // the period
    if( duty_cycle_int >= period_int){
        duty_cycle_int = period_int/2;
        // converting long to char data type
        sprintf(duty_cycle_str, "%ld", duty_cycle_int);
        // setup dummy duty cycle
        set_pwm_duty_cycle(pwmchip, channel, duty_cycle_str);
    }
    // export file path
    char PWMPeriod[60];
    sprintf(PWMPeriod, "/sys/class/pwm/%s/pwm-4:%s/period", pwmchip, channel);
    fp = fopen(PWMPeriod, "w");
    fwrite(period, sizeof(char), strlen(period), fp);
    fclose(fp);
}

void start_pwm(char* pin_number, char* pwmchip, char* channel, char* period, char*
duty_cycle){
    /* Input:
    pin_number: pin_number to generate PWM on pwmchip: the device folder to generate PWM
    channel: pwm device channel perod: pwm period duty_cycle: pwm duty cycle */
    // configure the pin in PWM mode
    config_pin(pin_number, "pwm");
    // export PWM device
    FILE* fp;
    char PWMExport[40];
    sprintf(PWMExport, "/sys/class/pwm/%s/export", pwmchip);
    fp = fopen(PWMExport, "w");
    fwrite(channel, sizeof(char), sizeof(channel), fp);
    fclose(fp);
    // configure PWM Period
    set_pwm_period(pwmchip, channel, period);
    // configure PWM Duty Cycle
    set_pwm_duty_cycle(pwmchip, channel, duty_cycle);
    // enable PWM
    char PWMEnable[40];
    sprintf(PWMEnable, "/sys/class/pwm/%s/pwm-4:%s/enable", pwmchip, channel);
    // configure generating PWM
    fp = fopen(PWMEnable, "w");
    fwrite("1", sizeof(char), 1, fp);
    fclose(fp);
}

void stop_pwm(char* pin_number, char* pwmchip, char* channel){
    char PWMDisable[40];
    sprintf(PWMDisable, "/sys/class/pwm/%s/pwm-4:%s/enable", pwmchip, channel);
    // stop generating PWM
    FILE* fp = fopen(PWMDisable, "w");
    fwrite("0", sizeof(char), 1, fp);
    fclose(fp);
}

void configure_gpio_input(int gpio_number){
    char gpio_num[10];
    sprintf(gpio_num, "export%d", gpio_number);
    const char* GPIOExport="/sys/class/gpio/export";
    
    // exporting the GPIO to user space
    FILE* fp = fopen(GPIOExport, "w");
    fwrite(gpio_num, sizeof(char), sizeof(gpio_num), fp);
    fclose(fp);
    
    // setting gpio direction as input
    char GPIODirection[40];
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", gpio_number);
    // setting GPIO as input
    fp = fopen(GPIODirection, "w");
    fwrite("in", sizeof(char), 2, fp);
    fclose(fp);
}

int main(){
    // configure pin P8_8 as input with internal pull-up enabled
    int gpio_number1 = 67;
    int gpio_number2 = 69;
    configure_gpio_input(gpio_number1);
    configure_gpio_input(gpio_number2);
    // file path to read button status
    char valuePath1[40];
    char valuePath2[40];
    sprintf(valuePath1, "/sys/class/gpio/gpio%d/value", gpio_number1);
    sprintf(valuePath2, "/sys/class/gpio/gpio%d/value", gpio_number2);
    // wait before first readings to avoid faulty readings
    sleep(1);
    int state1;
    int state2;
    FILE *fp1;
    FILE *fp2;
    char pin_number[32] = "P9_16";
    char pwmchip[32] = "pwmchip4";
    char channel[32] = "1";
    char period1[32] = "100000000";
    char period2[32] = "1000000";
    char duty_cycle[32] = "50000000";
    // loop to monitor events
    while(1){

        fp1 = fopen(valuePath1, "r");
        // default state is 1 since buttons are configured with
        // internal pull-up resistors. So, reading 0 means button
        // is pressed
        fscanf(fp1, "%d", &state1);
        fclose(fp1);
        
        fp2 = fopen(valuePath2, "r");
        fscanf(fp2, "%d", &state2);
        fclose(fp2);
        // button 1 pressed
        if( state1 == 0 ){
            start_pwm(pin_number, pwmchip, channel, period1, duty_cycle); // start pwm
            //connect these pins to the speaker instead of the pintf function Set PWM to 10Hz, 50% duty cycle

        }else if(state2 == 0){
            start_pwm(pin_number, pwmchip, channel, period2, duty_cycle); // start pwm
            // set PWM to 1000Hz, 50% duty cycle
        }else{
            stop_pwm(pin_number, pwmchip, channel); // Make sure the pwm pin and channel are cleared first
        }
    }
    return 0;
}