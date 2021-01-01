#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include "batt.h"

#define VOLTAGE_FLAG 0
#define PERCENT_FLAG 1

int set_batt_from_ports(batt_t *batt){
  if(BATT_VOLTAGE_PORT<0){
    return 1;  //return 1 as an error
  }

  batt->volts = BATT_VOLTAGE_PORT;    //set the volts
  if(batt->volts >= 3800){            //set for the percent value maximum and minimum and current
    batt->percent = 100;
  }
  else if(batt->volts <=3000){
    batt->percent = 0;
  }
  else{
    batt->percent = (BATT_VOLTAGE_PORT - 3000)/8;
  }

  if(BATT_STATUS_PORT & 0x1){   //set the mode
    batt->mode = 1;
  }
  else{
    batt->mode = 0;
  }
  return 0;
}

int set_display_from_batt(batt_t batt, int *display){
  *display = 0b0;
  int mask[10]={0b0111111, 0b0000011, 0b1101101, 0b1100111, 0b1010011, 0b1110110, 0b1111110, 0b0100011, 0b1111111, 0b1110111}; //set the mask
  int batt_right_vol = (batt.volts % 100);	// to get right digit of volts
  if(batt_right_vol%10>=5){     //if bigger than 5, round up
        batt_right_vol = (batt_right_vol/10)+1;
  }
  else{
    batt_right_vol = batt_right_vol/10;
  }
  int batt_middle_vol = (batt.volts % 1000)/100;	// to get middle digit of volts
  int batt_left_vol = (batt.volts % 10000)/1000; //to get left digit of volts

  int batt_right_per = (batt.percent %10)/1; //to get right digit of percentage
  int batt_middle_per = (batt.percent %100)/10; //to get middle digit of percentage
  int batt_left_per = (batt.percent %1000)/100; //to get left digit of percentage

  if(batt.mode == VOLTAGE_FLAG){  //check if it's voltage or not
    *display |= 0b1<<22;  //set the voltage signal
    *display |= 0b1<<21; //set the decimal signal
    *display |= mask[batt_right_vol]; //setting the right side number
  }
  else if(batt.mode == PERCENT_FLAG){ //check if it's percent or not
    *display |= 0b1<<23; //set the percentage signal
    *display |= mask[batt_right_per]; //set the right side number
  }
  if(batt.mode == VOLTAGE_FLAG && batt_left_vol > 0){
    *display |= mask[batt_left_vol]<<14;  //setting the left number
  }
  if(batt.mode == VOLTAGE_FLAG && batt_middle_vol >= 0){
    *display |= mask[batt_middle_vol]<<7;  //setting the middle number
  }
  if(batt.mode == PERCENT_FLAG && batt_left_per > 0){
    *display |= mask[batt_left_per]<<14;  //setting the left number
  }
  if(batt.mode == PERCENT_FLAG && batt_left_per != 0 && batt_middle_per >=0){
    *display |= mask[batt_middle_per]<<7;
  }
  else if(batt.mode == PERCENT_FLAG && batt_left_per == 0 && batt_middle_per > 0){
    *display |= mask[batt_middle_per]<<7;  //setting the middle number
  }
  if(batt.percent >=5 && batt.percent<=29){         //setting for the battery display
    *display |= 0b1<<28;
  }
  else if(batt.percent >=5 && batt.percent<=49){
    *display |= 0b1<<28;
    *display |= 0b1<<27;
  }
  else if(batt.percent >=5 && batt.percent<=69){
    *display |= 0b1<<28;
    *display |= 0b1<<27;
    *display |= 0b1<<26;
  }
  else if(batt.percent >=5 && batt.percent<=89){
    *display |= 0b1<<28;
    *display |= 0b1<<27;
    *display |= 0b1<<26;
    *display |= 0b1<<25;
  }
  else if(batt.percent >=5 && batt.percent>89){
    *display |= 0b1<<28;
    *display |= 0b1<<27;
    *display |= 0b1<<26;
    *display |= 0b1<<25;
    *display |= 0b1<<24;
  }
  return 0;
}

int batt_update(){
  batt_t temp;
  int display;
  if(set_batt_from_ports(&temp) != 0){ //if there is an error
    return 1;
  }
  set_display_from_batt(temp, &display);
  BATT_DISPLAY_PORT = display;
  return 0;
}
