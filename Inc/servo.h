#ifndef SERVO_H_   /* Include guard */
#define SERVO_H_

#include "stdint.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim8;

TIM_HandleTypeDef* t1 = &htim1;
TIM_HandleTypeDef* t2 = &htim2;
TIM_HandleTypeDef* t3 = &htim8;
TIM_HandleTypeDef* t4 = &htim4;
TIM_HandleTypeDef* t5 = &htim5;

uint32_t ch1 = TIM_CHANNEL_1;
uint32_t ch2 = TIM_CHANNEL_2;
uint32_t ch3 = TIM_CHANNEL_3;
uint32_t ch4 = TIM_CHANNEL_4;

/*
			pwm values   									FRONT									Leg Servo Format
			25  ->								E14	 _ _ _ _ _ _	E9					- Body -
			50										D14	|						|	E11					> Body Servo  E9, E14, C6, B10
			75  ^									D15	|						|	E13					> Joint Servo E11, D14, C7, B11
			100									    	|           |							> Leg Servo   E13, D15, C9, A2
			125 <-										|           |
																|           | 
														C6	|           | A2
														C7	|_ _ _ _ _ _| B10
														C9								B11
																		BACK								
		*/


void pwm_set(TIM_HandleTypeDef* timer, int channel, uint16_t value);

inline void wait (){HAL_Delay(1000);}


 void pwm_set(TIM_HandleTypeDef* timer, int channel, uint16_t value){
    if (channel == 1) { timer -> Instance -> CCR1 = value; return;}
    if (channel == 2) { timer -> Instance -> CCR2 = value; return;}
    if (channel == 3) { timer -> Instance -> CCR3 = value; return;}
    if (channel == 4) { timer -> Instance -> CCR4 = value; return;}   
}	
 
void move(const char* pin, uint16_t value){
	if (strcmp(pin,"E14") == 0){pwm_set(t1, 4, value);}
	if (strcmp(pin,"D14") == 0){pwm_set(t4, 3, value);}
	if (strcmp(pin,"D15") == 0){pwm_set(t4, 4, value);}
	
	if (strcmp(pin,"C6") == 0){pwm_set(t3, 1, value);}
	if (strcmp(pin,"C7") == 0){pwm_set(t3, 2, value);}
	if (strcmp(pin,"D13") == 0){pwm_set(t4, 2, value);}
	if (strcmp(pin,"E9") == 0){pwm_set(t1, 1, value);}
	if (strcmp(pin,"E11") == 0){pwm_set(t1, 2, value);}
	if (strcmp(pin,"E13") == 0){pwm_set(t1, 3, value);}
	
	if (strcmp(pin,"A2") == 0){pwm_set(t5, 3, value);}
	if (strcmp(pin,"B10") == 0){pwm_set(t2, 3, value);}
	if (strcmp(pin,"B11") == 0){pwm_set(t2, 4, value);}
}


 void servo_stand(){
   
		int delay = 100;
	 
   // pwm_set(t1, 4, 50);  //E14	50	
		move("E14",50);
    HAL_Delay(delay);
		move("D14",100);
	//	pwm_set(t4, 3, 100); //D14	100
		HAL_Delay(delay);
		move("D15",100);
		//pwm_set(t4, 4, 100); //D15	100
		HAL_Delay(delay);
 
    pwm_set(t3, 1, 100); 	//C6	100
    HAL_Delay(delay);
		pwm_set(t3, 2, 100); 	//C7	100
		HAL_Delay(delay);
		pwm_set(t4, 2, 100); 	//D13	100
		HAL_Delay(delay);
    
    pwm_set(t1, 1, 100); 	//E9	100
    HAL_Delay(delay);
		pwm_set(t1, 2, 50);  //E11 	50
		HAL_Delay(delay);
		pwm_set(t1, 3, 50);  //E13	50
		HAL_Delay(delay);
    
    pwm_set(t5, 3, 50);  //A2		50
    HAL_Delay(delay);
		pwm_set(t2, 3, 50);  //B10	50
		HAL_Delay(delay);
		pwm_set(t2, 4, 50);  //B11	50
		HAL_Delay(delay);
    
    for (int i = 0; i < 13; i++){
			
    pwm_set(t4, 3, 100 - i); //D14 100
		pwm_set(t4, 4, 100 - i); //D15 100
		pwm_set(t3, 2, 100 - i); //C7	 100
		pwm_set(t4, 2, 100 - i); //D13	 100
    pwm_set(t1, 2, 50 + i); //E11 50
		pwm_set(t1, 3, 50 + i); //E13 50
    pwm_set(t2, 3, 50 + i); //B10 50
		pwm_set(t2, 4, 50 + i); //B11 50
			
		HAL_Delay(100);
    }
}
 

 
 void servo_stop(){
	 return;
 }
 


void servo_turn_right(int turns){
	while (turns-- > 0){
	int delay = 100;
	pwm_set(t4, 3, 100); //D14 100
	HAL_Delay(delay);
	pwm_set(t1, 4, 25); //E14 25
	HAL_Delay(delay);
	pwm_set(t4, 3, 85); //D14 85
	HAL_Delay(delay);
	
	
	pwm_set(t1, 2, 50); //E11 50
	HAL_Delay(delay);
	pwm_set(t1, 1, 75); //E9  75
	HAL_Delay(delay);
	pwm_set(t1, 2, 65); //E11 65
	HAL_Delay(delay);
		
	pwm_set(t2, 3, 50); //B10 50
	HAL_Delay(delay);
	pwm_set(t5, 3, 25); //B10 65
	HAL_Delay(delay);
	pwm_set(t2, 3, 65); //B10 65
	HAL_Delay(delay);
	
	pwm_set(t3, 2, 100); //C7 100
	HAL_Delay(delay);
	pwm_set(t3, 1, 75); //C6 75
	HAL_Delay(delay);	
	pwm_set(t3, 2, 85); //C7 85
	HAL_Delay(delay);
	
	for (int i = 0; i < 25; i += 5){
	 pwm_set(t1, 4, 25 + i);
	 pwm_set(t1, 1, 75+ i);
	 pwm_set(t5, 3, 25+ i);
	 pwm_set(t3, 1, 75 + i);
		HAL_Delay(50);
	}
			HAL_Delay(500);
	}
}  

void servo_turn_left(int turns){
	while (turns-- > 0){
	int delay = 100;
	
	pwm_set(t4, 3, 100);//D14 100
	HAL_Delay(delay);
	pwm_set(t1, 4, 75); //E14 25
	HAL_Delay(delay);
	pwm_set(t4, 3, 85); //D14 85
	HAL_Delay(delay);
	
	pwm_set(t1, 2, 50); //E11 50
	HAL_Delay(delay);
	pwm_set(t1, 1, 125); //E9  75
	HAL_Delay(delay);
	pwm_set(t1, 2, 65); //E11 65
	HAL_Delay(delay);
	
	pwm_set(t2, 3, 50); //B10 50
	HAL_Delay(delay);
	pwm_set(t5, 3, 75); //A2 25
	HAL_Delay(delay);
	pwm_set(t2, 3, 65); //B10 65
	HAL_Delay(delay);
	
	pwm_set(t3, 2, 100); //C7 100
	HAL_Delay(delay);
	pwm_set(t3, 1, 125); //C6 75
	HAL_Delay(delay);
	pwm_set(t3, 2, 85); //C7 85
	HAL_Delay(delay);
/*	
	 pwm_set(t1, 4, 50);
	 pwm_set(t1, 1, 100);
	 pwm_set(t5, 3, 50);
	 pwm_set(t3, 1, 100);
	 */
	 for (int i = 0; i < 25; i += 5){
	 pwm_set(t1, 4, 75 - i);
	 pwm_set(t1, 1, 125 - i);
	 pwm_set(t5, 3, 75 - i);
	 pwm_set(t3, 1, 125 - i);
		HAL_Delay(50);
	}
	
			HAL_Delay(500);
	}
}
 
void servo_init_walking(){
	int delay = 100;
	 int x = 12;
   int y = 10;

	// Initiate Walking Stance

   move("D14", 100);
   HAL_Delay(delay);
   move("E14", 75);
   move("D15", 85);
   HAL_Delay(delay);
   move("D14", 85);
   
   HAL_Delay(delay*2);
   
   move("C7", 100);
   HAL_Delay(delay);
   move("C6", 75);
   move("D13", 85);
   HAL_Delay(delay);
   move("C7", 85);
   
   HAL_Delay(delay*2);
   
   move("E11", 50);
   HAL_Delay(delay);
   move("E9", 75 + x);
   move("E13", 65);
   HAL_Delay(delay);
   move("E11", 65);
   
   HAL_Delay(delay*2);
   
   move("B10", 50);
   HAL_Delay(delay);
   move("A2", 75 - x);
   move("B11", 65);
   HAL_Delay(delay);
   move("B10", 65);
   
   HAL_Delay(delay*2);

}



void servo_walking_forward(){
   int delay = 100;
	 int x = 12;
   int y = 10;

// Walking Animation
		
		// Looping Walking with While Loop
   move("D14", 100);
   HAL_Delay(delay);
   move("E14", (75 - 2 * x));
   move("D15", (85 - y));
   HAL_Delay(delay);
	 move("D14", 85);

   HAL_Delay(delay*2);
   
   move("E14", (75 - x));
   move("D15", 85);
   
   move("C6", (75 + x)); 
   move("D13", 85);
   
   move("E9", 75);
   move("E13", 65);
   
   move("A2", (75 - 2 * x));
   move("B11",(65 + y));
   
   HAL_Delay(delay*2);
   
   move("B10", 50);
   HAL_Delay(delay);
   move("A2", 75);
   move("B11", 65);
   HAL_Delay(delay);
   move("B10", 65);
          
   HAL_Delay(delay*2);
   
  
   move("E11", 50);
   HAL_Delay(delay);
   move("E9", (75 + 2 * x));
   move("E13", (65 + y));
   HAL_Delay(delay);
   move("E11", 65);
   
   HAL_Delay(delay*2);
   
   move("E14", 75);
   move("D15", 85);
   
   move("C6", (75 + 2 * x));
   move("D13", (85 - y));
   
   move("E9", (75 + x));
   move("E13", 65);
   
   move("A2", (75 - x));
   move("B11", 65);
   
   HAL_Delay(delay*2);
   
   move("C7", 100);
   HAL_Delay(delay);
   move("C6", 75);
   move("D13", 85);
   HAL_Delay(delay);
   move("C7", 85);
   
   HAL_Delay(delay*2);
   
	 // Revert to Start-Up Stance
	 //servo_stand(); 
}




 
 #endif // SERVO_H_