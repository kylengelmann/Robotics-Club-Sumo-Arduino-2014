#include "sumo.h"
#include <Wire.h>
#include <LSM303.h>
#include <QTRSensors.h>
#include <ZumoMotors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <Arduino.h>


static int STATE = STATE_START;

void setup()
{
	Serial.begin(9600);

	accel.setup();

}


ZumoBuzzer buzzer;
Pushbutton button(ZUMO_BUTTON);

void waitForStart()
{
	button.waitForButton();

	for (int i = 0; i < 5; i++)
	{
		delay(1000);
		buzzer.playNote(NOTE_G(3), 200, 15);
	}
}


void loop()
{

	//waitForStart();
	robotMotor.arc(DIR_RIGHT,35);

	while(1){

		// Update motors
		// Take sensor readings
		//accel.update();
		refl.update();
		front_ir.update();
		back_ir.update();

		if(refl.onEdge())
		{
			Serial.println("HIT LINE");
		}

		// State actions
		switch(STATE){
			case STATE_START: 
				// Begin arching
				robotMotor.arc(DIR_RIGHT, 35);

				STATE = STATE_LOCATE;
			        break;
			case STATE_LOCATE:  // Eventually all other states should go back to this one once they are done


				// Check IR sensor for other robot

				if(front_ir.objectAhead()){ // Found straight ahead
					STATE = STATE_ATTACK;
				}
				else if(back_ir.objectAhead()){ // Found behind
					// Modify motot arching
					robotMotor.arc(DIR_LEFT, 5);
				}
                                else if(right_ir.objectAhead()){ // Found behind
					// Modify motot arching
					robotMotor.arc(DIR_RIGHT, 5);
				}
                                else if(left_ir.objectAhead()){ // Found behind
					// Modify motot arching
					robotMotor.arc(DIR_LEFT, 5);
				}
			        break;
			case STATE_ATTACK: 
				// Track robot and move towards it
				if(!front_ir.objectAhead()){
					STATE = STATE_START;
				}
				else
					robotMotor.forward();
			        break;
			case STATE_ENGAGE: 
				// Go straight on full power
				robotMotor.forward();
			        break;
			case STATE_DEFEND: 

				// Depending on the collision angle, try to evade

			        break;
			case STATE_EDGE: 
				// Depending on which reflectance sensor was hit, turn to not fall off

				if(refl.onEdge() == DIR_RIGHT){
					robotMotor.arc(DIR_LEFT, 0);
				}
				else if(refl.onEdge() == DIR_LEFT){
					robotMotor.arc(DIR_RIGHT, 0);
				}
				else{
					delay(100);
					STATE = STATE_START;
				}

			      break;
		}


		// Special case state transitions

		if(refl.onEdge()){ // Edge detected
			STATE = STATE_EDGE;
		}

		if(accel.collided()){ // Collision

			if(accel.getDirection() == DIR_FRONT){ // Head-on
				STATE = STATE_ENGAGE;
			}
			else{
				STATE = STATE_DEFEND;
			}
		}


		// Sleep
		delay(33);
	}

}
