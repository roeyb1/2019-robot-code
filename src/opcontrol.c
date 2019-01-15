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

	--- SENSORS ---
	SorterEncoder = 1/2

*/

#define M_FRONT_LEFT 2
#define M_FRONT_RIGHT 3
#define M_BACK_LEFT 4
#define M_BACK_RIGHT 5
#define PICKUP 6
#define SHOOTER 7
#define RAMP 8
#define SORTER 9

#define DEADZONE 50

int pickupIsActive = 0;

void moveRobot();
void stopRobot();

void operatorControl() {
	while (1) {

		// Drive
		if (abs(joystickGetAnalog(1,3)) > DEADZONE || abs(joystickGetAnalog(1,4)) > DEADZONE || abs(joystickGetAnalog(1,1)) > DEADZONE)
			moveRobot();
		else
			stopRobot();
		// End drive

		// Pickup
		if (digitalRead(4))
			pickupIsActive = 0;
		else
			pickupIsActive = 1;

		if (joystickGetDigital(1, 7, JOY_LEFT))
		{
			motorSet(PICKUP, -127);
			digitalWrite(3, HIGH);
		}
		else if (pickupIsActive)
		{
			motorSet(PICKUP, 127);
			digitalWrite(3, LOW);
		}
		// End pickup


		// Shooter
		if (joystickGetDigital(1, 5, JOY_DOWN))
			motorSet(7, 127);
		else if (joystickGetDigital(1, 5, JOY_UP))
			motorStop(7);
		// End shooter

		// Ramp
		if (joystickGetDigital(1, 6, JOY_UP))
			motorSet(8, 127);
		else if (joystickGetDigital(1, 6, JOY_DOWN))
			motorSet(8, -127);
		else
			motorStop(8);
		// End ramp

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
