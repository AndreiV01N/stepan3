## Hardware ##

- Arduino MEGA 2560 r3
- IMU/AHRS: any of GY-85, GY-521, GY-87
- ESP-01 (ESP8266)
- HC-SR04
- CNC Shield
- 2x A4988 drivers
- 2x NEMA 17HS4401
- 2x 96-mm aluminium wheels w/ 6-mm D-shaft holes
- 6x AA alkaline batteries in a flat battery case


## Software ##

The main program code runs on robot's arduino board. It is placed in arduino/ directory.
The robot can also be controlled and tuned remotely through wi-fi using a simple android app.

**Changes since initial commit:**

- In addition to MPU6050's DMP engine it's been implemented open source AHRS filters (Madgwick/Mahony)
- Added support for some other IMU breakout boards (GY-85, GY-87)
- Onboarded Ultrasonic HC-SR04
- AA's charge control has also been added
- Added useful calibrating and debugging tools (see HOWTOs)
- Optimized DC stepper motors' timers settings as well as speed scaling, improved jumping up mechanics
