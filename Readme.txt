STEP CLOCK by Makit
READ LICENSE FILE BEFORE USING IT

IMPORTANT NOTE: Please, bear in mind that this code is a beta version. It works but it may have some bugs/errors.

This is the code that runs my 3D printed Step Clock.
STL files can be downloaded from here: https://pinshape.com/items/36893-3d-printed-step-clock

Visit <http://www.makitpro.com> for more information and instructions about the project.

LIBRARIES
You’ll need three libraries:
- RTClib (https://github.com/adafruit/RTClib)
- Accelstepper (http://www.airspayce.com/mikem/arduino/AccelStepper/)
- wire.h that is already included with Arduino IDE

VERSIONS
There is a birthday version with a buzzer that will play a song the day you choose. Just change the value of birthdayDay, birthdayMonth and birthdayHour variables to match your birthday. Of course you'll need to add a buzzer.
IMPORTANT: if you choose this version, the code will play the song and stop working at the set time and you'll need to reset the board with the reset button.

SET TIME
The first time you use it, uncomment on of these two lines:

//rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));//this sets time manually
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//this sets time based on the compilation time of the computer

Then comment again and upload the sketch. Once the time is set, the RTC module will keep it synced as long as it has battery.

CALIBRATION

You’ll also need to calibrate the zero position. The code works like this: first minute motor moves until it detects the magnet and then hour motor does the same. At this moment, calibration of zero position (12 o’clock) has to be done. You have two ways of calibrating, with code or manually. The code is set to adjust them manually by default but you can change it commenting or uncommenting these lines:

//Code calibration, change these values if necessary
stepperMin1.runToNewPosition(3675);//Manual calibration, change this value to achieve 0 position
stepperHour1.runToNewPosition(3900);//Manual calibration, change this value to achieve 0 position

//Manual calibration, comment if you are using code calibration or hands are already in place
Serial.println("Insert minute and hour hand on the 12 o'clock position, you have 20 seconds");
delay(20000);//Time to put the hour and minute hands on the correct position, you can change it
With manual calibration, the code pauses (20 seconds by default) to give you time to insert the hands in the correct place (12 o’clock).

With code calibration, the minute and hour motors move “some steps” (3675 and 3900 in the exemple) until they reach the o’clock position. These “some steps” is what you may have to change if necessary.

Just bear in mind that to make one complete turn, the stepper motor has to move 4096 steps. If you need half turn, then correct the zero position with 2048 steps, a quarter with 1024 and so on.

If you find yourself with the motor turning counter-clockwise, just add a minus (-) before the steps. For instance, -4096 will move one turn counter-clockwise and 4096 clockwise or vice-versa depending on your connections.
