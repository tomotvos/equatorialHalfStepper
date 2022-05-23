// Includes the Arduino Stepper Library
#include <AccelStepper.h>

// Defines the number of steps per rotation...this is purely a function of the stepper motor
const int stepsPerRevolution = 2 * 2048;

// Define the final gear ratio between the drive bearing and the radius of rotation
// ...for now, the units are inches, for simplicity
const float driveBearingRadius = 0.25;
const float axisOfRotationRadius = 42.875 / 2.0;
const float eqGearRatio = axisOfRotationRadius / driveBearingRadius;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper myStepper = AccelStepper(AccelStepper::HALF4WIRE, 8, 10, 9, 11);

// We'll need these accessible but modifiable
float siderealMotorRpm = 0;
float siderealPerMinute = 0;
int microsecondsPerStep = 0;
int millisecondsPerStep = 0;

void setup()
{
	Serial.begin(115200);

	// For this library, it is all about calculating the right speed since we just want one rate
	// ...note that the speed is in steps/sec

	float siderealPerMinute = 1.0 / (23.0 * 60.0 + 56.0);
	float siderealMotorRpm = siderealPerMinute * eqGearRatio;
	float stepsPerSecond = siderealMotorRpm * (float)stepsPerRevolution / 60.0;
	Serial.println(stepsPerSecond, 4);
	Serial.flush();

	// for debugging, we divide out the gear ratio so our stepper goes at the sidereal rate...this will ensure
	// it goes 15 degrees per hour...by commenting this out, it turns at the correct rate for the gear ratio
	// implied by the bearing diameter and axis of rotation
	// stepsPerSecond /= eqGearRatio;

	// ultra-debugging!
	// stepsPerSecond = 100.0;

	// alternative debugging, just speed things up a bit
	// stepsPerSecond *= 10.0;

	if (stepsPerSecond > 1.0)
		myStepper.setMaxSpeed(stepsPerSecond);

	// the motor is on the NW bearing, so we want to be *pushing*, thus negative
	myStepper.setSpeed(-stepsPerSecond);
}

void loop()
{
	// The main loop merely calls runSpeed() as often as it can, to keep it going at the constant rate we set
	myStepper.runSpeed();
}
