// Includes the Arduino Stepper Library
#include <AccelStepper.h>

// #def NEMA_17

#ifdef NEMA_17

// Defines the number of steps per rotation...this is purely a function of the stepper motor
// ...we wire up the NEMA-17 to micro-step, so stride angle needs to be adjusted for that
const float microStep = 1.0 /* 16.0 */;
const float strideAngle = 1.8 / microStep;
// using this as a fudge factor while the stepping issues are worked out
const float wormGearRatio = 1.2;
// this controls the primary direction of rotation...is motor driving roller directly or geared?
const bool directDrive = true;

// Creates an instance of stepper class
// Just need two pins, for STEP and DIR, and the controller looks after the rest, including micro-stepping
AccelStepper myStepper = AccelStepper(AccelStepper::DRIVER, 3, 2);

#else

// Defines the number of steps per rotation...this is purely a function of the stepper motor
// ...the 28BYJ-48 has 64:1 internal gear, so stride angle needs to be adjusted for that
const float strideAngle = 5.625 / 64.0;
// this controls the primary direction of rotation...is motor driving roller directly or geared?
const bool directDrive = true;
const float wormGearRatio = directDrive ? 1.0 : 40.0;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper myStepper = AccelStepper(AccelStepper::HALF4WIRE, 8, 10, 9, 11);

#endif

const float stepsPerRevolution = 360.0 / strideAngle;

// Define the final gear ratio between the drive bearing and the radius of rotation
// ...for now, the units are inches, for simplicity
const float driveBearingRadius = 0.25;
const float axisOfRotationRadius = 42.875 / 2.0;
const float eqGearRatio = axisOfRotationRadius / driveBearingRadius;

long g_initialPosition = 0;

void setup()
{
	Serial.begin(115200);

	// For this library, it is all about calculating the right speed since we just want one rate
	// ...note that the speed is in steps/sec

	float siderealPerMinute = 1.0 / (23.0 * 60.0 + 56.0);
	float siderealMotorRpm = siderealPerMinute * eqGearRatio;
	float stepsPerSecond = siderealMotorRpm * stepsPerRevolution / 60.0 * wormGearRatio;
	
	// for debugging, we divide out the gear ratio so our stepper goes at the sidereal rate...this will ensure
	// it goes 15 degrees per hour...by commenting this out, it turns at the correct rate for the gear ratio
	// implied by the bearing diameter and axis of rotation
	// stepsPerSecond /= eqGearRatio;

	// ultra-debugging!
	// stepsPerSecond = 1.0;

	// alternative debugging, just speed things up a bit
	stepsPerSecond *= 4.0;

	// for the rocking test...
	g_initialPosition = myStepper.currentPosition();

	Serial.print("EQ gear ratio: ");
	Serial.println(eqGearRatio, 4);
	Serial.print("Steps per second: ");
	Serial.println(stepsPerSecond, 4);
	Serial.flush();

	if (stepsPerSecond > 1.0)
		myStepper.setMaxSpeed(stepsPerSecond);

	// the motor is on the NW bearing, so we want to be *pushing*, thus negative
	// ... also note that because of the orientation of the motors, the second motor is
	// ... wired to go in reverse even though it "steps" through single myStepper

	// ... still on NW, but now worm gear has reversed direction so no need for negative
	// ... we now use directDrive to determine whether to flip or not
	myStepper.setSpeed((directDrive ? -1.0 : 1.0) * stepsPerSecond);

	blink(3);
}

void blink(int numBlinks)
{
	if (numBlinks <= 1)
		numBlinks = 1;

	for (int i = 1; i <= numBlinks; i++)
	{
		delay(1000);
		digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
	}
}

void rockAndRoll()
{
	const long rockAmplitudeSteps = directDrive ? 500 : 100000;

	if (abs(myStepper.currentPosition() - g_initialPosition) > rockAmplitudeSteps)
	{
		myStepper.setSpeed(myStepper.speed() * -1.0);
		Serial.println("Changing step direction");
		Serial.flush();
		blink(2);
	}
}

void loop()
{
	// for testing torque
	// rockAndRoll();

	// The main loop merely calls runSpeed() as often as it can, to keep it going at the constant rate we set
	myStepper.runSpeed();
}
