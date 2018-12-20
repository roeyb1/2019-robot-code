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

#define DEADZONE 50

/*
	FrontLeft = 2,
	FrontRight = 7,
	BackLeft = 3,
	BackRight = 8,
*/

void moveRobot();
void stopRobot();

void operatorControl() {
	while (1) {
		if (abs(joystickGetAnalog(1,3)) > DEADZONE || abs(joystickGetAnalog(1,4)) > DEADZONE || abs(joystickGetAnalog(1,1)) > DEADZONE)
			moveRobot();
		else
		stopRobot();
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

	motorSet(2, frontLeftPower);
	motorSet(7, frontRightPower);
	motorSet(3, backLeftPower);
	motorSet(8, backRightPower);
}

void stopRobot()
{
	motorStopAll();
}
