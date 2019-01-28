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
	Lifter_limit = 3

*/

#define M_FRONT_LEFT 2
#define M_FRONT_RIGHT 3
#define M_BACK_LEFT 4
#define M_BACK_RIGHT 5
#define PICKUP 1
#define SHOOTER 7
#define RAMP 8
#define SORTER 9
#define LIFTER 6

#define DEADZONE 50

int pickupIsActive = 0;
int lifterAtMax = 0;
int lifterAtMin = 0;
int sorterIsActive = 0;

unsigned long pickupLastTime = 0;
unsigned long sorterLastTime = 0;
unsigned long debounceDelay = 100;

void moveRobot();
void stopRobot();
void handlePickup(unsigned char buttonGroup, unsigned char button);
void sortFriendly();
void sortEnemy();

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
			motorSet(SHOOTER, 127);
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
		if (digitalRead(3) == LOW)
			lifterAtMax = 1;
		else
			lifterAtMax = 0;
		if (digitalRead(4) == LOW)
			lifterAtMin = 1;
		else
			lifterAtMin = 0;

		if (joystickGetDigital(1,8,JOY_UP) && !lifterAtMax)
			motorSet(LIFTER, 127);
		else if (joystickGetDigital(1,8,JOY_DOWN) && !lifterAtMin)
			motorSet(LIFTER, -127);
		else
			motorStop(LIFTER);
		// End lifter


		// Sorter
		if (joystickGetDigital(1,8, JOY_LEFT))
		{
			sorterIsActive = 1;
		}
		sortFriendly();
		// End sorter
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

void sortFriendly()
{
	if (sorterIsActive)
	{
		if (encoderGet(sorter) <= 90)
			motorSet(SORTER, 20);
		else if (encoderGet(sorter) > 90)
		{
			motorStop(SORTER);
			sorterIsActive = 0;
			encoderReset(sorter);
		}
	}
}
