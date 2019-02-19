/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"
#include <stdlib.h>

/*
 * Runs the user operator control code. This function will be started in its own task with the
 * default priority and stack size whenever the robot is enabled via the Field Management System
 * or the VEX Competition Switch in the operator control mode. If the robot is disabled or
 * communications is lost, the operator control task will be stopped by the kernel. Re-enabling
 * the robot will restart the task, not resume it from where it left off.
 *
 * If no VEX Competition Switch or Field Management system is plugged in, the VEX Cortex will
 * run the operator control task. Be warned that this will also occur if the VEX Cortex is
 * tethered directly to a computer via the USB A to A cable without any VEX Joystick attached.
 *
 * Code running in this task can take almost any action, as the VEX Joystick is available and
 * the scheduler is operational. However, proper use of delay() or taskDelayUntil() is highly
 * recommended to give other tasks (including system tasks such as updating LCDs) time to run.
 *
 * This task should never exit; it should end with some kind of infinite loop, even if empty.
 */

/*

	--- MOTORS ---
	FrontLeft = 2,
	FrontRight = 3,
	BackLeft = 4,
	BackRight = 5,
	Pickup = 6,
	Shooter = 7,
	Ramp = 8
	Sorter = 9
	Lifter = 10

	--- SENSORS ---
	SorterEncoder = 1/2
	Lifter max = 3
	Lifter min = 4
	Arduino = 5

*/

// Motor ports
#define M_FRONT_LEFT 2
#define M_FRONT_RIGHT 3
#define M_BACK_LEFT 4
#define M_BACK_RIGHT 5
#define PICKUP 1
#define SHOOTER 7
#define RAMP 8
#define SORTER 9
#define LIFTER 6
#define MIXER 10

//Sensor ports
#define LIFTER_SENS_MAX 3
#define LIFTER_SENS_MIN 4
#define ARDUINO_SENS_OUT 7

#define DEADZONE 20
#define MIXER_SPEED 30

int pickupIsActive = 0;
int lifterAtMax = 0;
int lifterAtMin = 0;
int sorterFriendly = 0;
int sorterEnemy = 0;

unsigned long pickupLastTime = 0;
unsigned long sorterLastTime = 0;
unsigned long debounceDelay = 100;

void moveRobot();
void stopRobot();
void handlePickup(unsigned char buttonGroup, unsigned char button);
void sort();
int getArduinoOut();

void operatorControl() {
	while (1) {
		// Drive
		if (abs(joystickGetAnalog(1,3)) > DEADZONE || abs(joystickGetAnalog(1,4)) > DEADZONE || abs(joystickGetAnalog(1,1)) > DEADZONE)
			moveRobot();
		else
			stopRobot();
		// End drive

		// Pickup
		if (joystickGetDigital(1,7,JOY_RIGHT))
		{
			handlePickup(7,JOY_RIGHT);
			if (pickupIsActive)
				motorSet(PICKUP, 127);
			else
				motorStop(PICKUP);
		}
		// End pickup


		// Shooter
		if (joystickGetDigital(1, 5, JOY_DOWN))
			motorSet(SHOOTER, 80);
		else if (joystickGetDigital(1, 5, JOY_UP))
			motorStop(SHOOTER);
		// End shooter

		// Ramp
		if (joystickGetDigital(1, 6, JOY_UP))
			motorSet(RAMP, 127);
		else if (joystickGetDigital(1, 6, JOY_DOWN))
			motorSet(RAMP, -127);
		else
			motorStop(RAMP);
		// End ramp

		// Lifter
		if (digitalRead(LIFTER_SENS_MAX) == LOW) // low when switch is pressed
			lifterAtMax = 1;
		else
			lifterAtMax = 0;
		if (digitalRead(LIFTER_SENS_MIN) == LOW) // low when switch is pressed
			lifterAtMin = 1;
		else
			lifterAtMin = 0;

		// Go up
		if (joystickGetDigital(1,8,JOY_UP) && !lifterAtMax)	
			motorSet(LIFTER, 127);
		// Go down
		else if (joystickGetDigital(1,8,JOY_DOWN) && !lifterAtMin) 
			motorSet(LIFTER, -127);
		else
			motorStop(LIFTER);
		// End lifter


		// Sorter
		// If either the manual input or the arduino input are set, set flags
		if ((joystickGetDigital(1,8, JOY_LEFT) || getArduinoOut() == 0) && !sorterEnemy)
			sorterFriendly = 1;
		else if ((joystickGetDigital(1,8, JOY_RIGHT) || getArduinoOut() == 1) && !sorterFriendly)
			sorterEnemy = 1;
		sort();
		// End sorter

		printf("Max: %d\n", lifterAtMax);
		printf("Min: %d\n", lifterAtMin);
		printf("------------\n");

		delay(20);
	}
}


void moveRobot()
{
	int control[3];
	control[0] = joystickGetAnalog(1,3);
	control[1] = joystickGetAnalog(1,1);
	control[2] = joystickGetAnalog(1,4);

	int frontLeftPower,
			frontRightPower,
			backLeftPower,
			backRightPower = 0;

	frontLeftPower = 0 - control[1] - control[0] + control[2];
	frontRightPower = 0 - control[1] + control[0] + control[2];
	backLeftPower = 0 - control[1] - control[0] - control[2];
	backRightPower = 0 - control[1] + control[0] - control[2];

	motorSet(M_FRONT_LEFT, frontLeftPower);
	motorSet(M_FRONT_RIGHT, frontRightPower);
	motorSet(M_BACK_LEFT, backLeftPower);
	motorSet(M_BACK_RIGHT, backRightPower);
}

void stopRobot()
{
	motorStop(M_FRONT_LEFT);
	motorStop(M_FRONT_RIGHT);
	motorStop(M_BACK_LEFT);
	motorStop(M_BACK_RIGHT);
}

void handlePickup(unsigned char buttonGroup, unsigned char button)
{
	if ((millis() - pickupLastTime) > debounceDelay)
	{
		pickupIsActive = !pickupIsActive;
		pickupLastTime = millis();
	}
}

void sort()
{
	if (sorterFriendly && (!sorterEnemy))
	{
		if (encoderGet(sorter) <= 90)
		{
			motorSet(SORTER, 20);
			motorSet(MIXER, MIXER_SPEED);
		}
		else if (encoderGet(sorter) > 90)
		{
			motorStop(SORTER);
			motorStop(MIXER);
			sorterFriendly = 0;
			encoderReset(sorter);
		}
	}
	else if (sorterEnemy && (!sorterFriendly))
	{
		if (encoderGet(sorter) >= -90)
		{
			motorSet(SORTER, -20);
			motorSet(MIXER, MIXER_SPEED);
		}
		else if (encoderGet(sorter) < -90)
		{
			motorStop(SORTER);
			motorStop(MIXER);
			sorterEnemy = 0;
			encoderReset(sorter);
		}
	}
}

int getArduinoOut()
{
	// If the arduino ouput pin is high, the ball is enemy team therefore return 1
	// otherwise return 0

	if (digitalRead(ARDUINO_SENS_OUT))
		return 1;
	else
		return 0;
}