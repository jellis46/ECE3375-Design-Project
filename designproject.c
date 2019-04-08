#include "address_map_arm.h"

void DisplayTime(int val, volatile char * display);
int getForceInput();

int main(void) {

    volatile int * timer = (int *)HPS_TIMER0_BASE;//main clock timer
    //set E to 0
    *(timer + 2) = 0;
    //set start value to 100 million
    *timer = 100000000;
    //set E and M to 1
    *(timer + 2) = 3;

    int time_s = 0;
    int time_10s = 0;
    int time_min = 0;
    int time_10min = 0;
    int time_hr = 0;
    int time_10hr = 0;

    int alarm_time_s = 5;
    int alarm_time_10s = 1;
    int alarm_time_min = 0;
    int alarm_time_10min = 0;
    int alarm_time_hr = 0;
    int alarm_time_10hr = 0;

    //displays
    volatile char * display_s = (char *)HEX3_HEX0_BASE;
    volatile char * display_10s = (char *)(HEX3_HEX0_BASE + 1);
    volatile char * display_min = (char *)(HEX3_HEX0_BASE + 2);
    volatile char * display_10min = (char *)(HEX3_HEX0_BASE + 3);
    volatile char * display_hr = (char *)HEX5_HEX4_BASE;
    volatile char * display_10hr = (char *)(HEX5_HEX4_BASE + 1);
	
    DisplayTime(time_s, display_s);
    DisplayTime(time_10s, display_10s);
    DisplayTime(time_min, display_min);
    DisplayTime(time_10min, display_10min);
    DisplayTime(time_hr, display_hr);
    DisplayTime(time_10hr, display_10hr);
    
    volatile int * LED_ptr = (int *)LED_BASE;

    int clockMode = 0; //0 = normal clock display, 1 = adjust alarm time
    int alarmSetSegment = 1; //control which time segment is being adjusted (1=s, 2=min, 3=hr)
    int alarmActive = 0;
	int ledStatus = 0;
	int ledCounter = 0;
    int alarmShutoffThreshold = 50;
	int buttonPressed = 0;
    
    volatile int * key_ptr = (int *)KEY_BASE;
	int temp;

    while(1) {
        if ((*((int *)(timer + 4)))&0x01) {//add to time when timer runs out
            time_s++;

            if (time_s == 10) {
                time_s = 0;
                time_10s++;
            }
            if (time_10s == 6) {
                time_10s = 0;
                time_min++;
            }
            if (time_min == 10) {
                time_min = 0;
                time_10min++;
            }
            if (time_10min == 6) {
                time_10min = 0;
                time_hr++;
            }
            if (time_hr == 10 || (time_hr == 4 && time_10hr == 2)) {
                time_hr = 0;
                time_10hr++;
            }
            if (time_10hr == 3) {
                time_10hr = 0;
            }

            if (clockMode == 0) {//update displays when in clock mode
                DisplayTime(time_s, display_s);
                DisplayTime(time_10s, display_10s);
                DisplayTime(time_min, display_min);
                DisplayTime(time_10min, display_10min);
                DisplayTime(time_hr, display_hr);
                DisplayTime(time_10hr, display_10hr);
            }

            if (time_s == alarm_time_s && time_10s == alarm_time_10s && time_min == alarm_time_min && time_10min == alarm_time_10min && time_hr == alarm_time_hr && time_10hr == alarm_time_10hr) {//activate alarm when alarm time equals clock time
                alarmActive = 1;
            }

	        *timer = 100000000;
            temp=*(timer + 3);
        }

        if (alarmActive == 1) {//flash LED when alarm is active
			ledCounter++;
			if (ledCounter == 50000) {
				if (ledStatus == 0)
				{
                    ledStatus = 1;
					ledCounter = 0;
                    *(LED_ptr) |= 0x1;
				}
				else
				{
                    ledStatus = 0;
					ledCounter = 0;
                    *(LED_ptr) &= ~0x1;
					
				}
			}

			if (getForceInput() >= alarmShutoffThreshold) {//disable alarm once force is over a cetain value
                alarmActive = 0;
                *(LED_ptr) &= ~0x1;
            }
        }

        if (*(key_ptr) == 1 && buttonPressed == 0) {//toggle clock mode

			buttonPressed = 1;
            if (clockMode == 0) {
                clockMode = 1;
                alarmSetSegment = 1;
                DisplayTime(alarm_time_s, display_s);
                DisplayTime(alarm_time_10s, display_10s);
                DisplayTime(alarm_time_min, display_min);
                DisplayTime(alarm_time_10min, display_10min);
                DisplayTime(alarm_time_hr, display_hr);
                DisplayTime(alarm_time_10hr, display_10hr);
            } else if (clockMode == 1) {
                clockMode = 0;
                DisplayTime(time_s, display_s);
                DisplayTime(time_10s, display_10s);
                DisplayTime(time_min, display_min);
                DisplayTime(time_10min, display_10min);
                DisplayTime(time_hr, display_hr);
                DisplayTime(time_10hr, display_10hr);
            }
                
        } else if (*(key_ptr) == 2 && buttonPressed == 0) {//change what time segment is being adjusted

			buttonPressed = 1;
			if (alarmSetSegment == 1) {
				alarmSetSegment = 2;
			} else if (alarmSetSegment == 2) {
				alarmSetSegment = 3;
			} else if (alarmSetSegment == 3) {
				alarmSetSegment = 1;
			}

        } else if (*(key_ptr) == 4 && clockMode == 1 && buttonPressed == 0) {//decrease time when in set alarm mode

			buttonPressed = 1;
            if (alarmSetSegment == 1) {//adjust seconds time
                alarm_time_s--;
                if (alarm_time_s == -1) {
                    alarm_time_s = 9;
                    alarm_time_10s--;
                }
                if (alarm_time_10s == -1) {
                    alarm_time_10s = 5;
                }
            } else if (alarmSetSegment == 2) {//adjust minutes time
                alarm_time_min--;
                if (alarm_time_min == -1) {
                    alarm_time_min = 9;
                    alarm_time_10min--;
                }
                if (alarm_time_10min == -1) {
                    alarm_time_10min = 5;
                }
            } else if (alarmSetSegment == 3) {//adjust hours time
                alarm_time_hr--;
                if (alarm_time_hr == -1) {
                    alarm_time_hr = 9;
                    alarm_time_10hr--;
                }
                if (alarm_time_10hr == -1) {
                    alarm_time_10hr = 2;
					alarm_time_hr = 3;
                }
            }
            DisplayTime(alarm_time_s, display_s);
            DisplayTime(alarm_time_10s, display_10s);
            DisplayTime(alarm_time_min, display_min);
            DisplayTime(alarm_time_10min, display_10min);
            DisplayTime(alarm_time_hr, display_hr);
            DisplayTime(alarm_time_10hr, display_10hr);

        } else if (*(key_ptr) == 8 && clockMode == 1 && buttonPressed == 0) {//increase time when in set alarm mode

			buttonPressed = 1;
            if (alarmSetSegment == 1) {//adjust seconds time
                alarm_time_s++;
                if (alarm_time_s == 10) {
                    alarm_time_s = 0;
                    alarm_time_10s++;
                }
                if (alarm_time_10s == 6) {
                    alarm_time_10s = 0;
                }
            } else if (alarmSetSegment == 2) {//adjust minutes time
                alarm_time_min++;
                if (alarm_time_min == 10) {
                    alarm_time_min = 0;
                    alarm_time_10min++;
                }
                if (alarm_time_10min == 6) {
                    alarm_time_10min = 0;
                }
            } else if (alarmSetSegment == 3) {//adjust hours time
                alarm_time_hr++;
                if (alarm_time_hr == 10 || (alarm_time_hr == 4 && alarm_time_10hr == 2)) {
                    alarm_time_hr = 0;
                    alarm_time_10hr++;
                }
                if (alarm_time_10hr == 3) {
                    alarm_time_10hr = 0;
                }
            }
            DisplayTime(alarm_time_s, display_s);
            DisplayTime(alarm_time_10s, display_10s);
            DisplayTime(alarm_time_min, display_min);
            DisplayTime(alarm_time_10min, display_10min);
            DisplayTime(alarm_time_hr, display_hr);
            DisplayTime(alarm_time_10hr, display_10hr);
        }
		
		if (*(key_ptr) != 1 && *(key_ptr) != 2 && *(key_ptr) != 4 && *(key_ptr) != 8) {
			buttonPressed = 0;
		}
    }
}

void DisplayTime(int val, volatile char * display) {
    unsigned char lookupTable [] = {0x3f, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,0x00};
    *(display) = lookupTable[val];
}

int getForceInput() {
    //simulate force using switches
	volatile int * num= (int *)SW_BASE;

	return *num;
}