#ifndef _PID_H
#define _PID_H

double position_PD_control(int32_t input, int32_t setpoint, double Kp, double Kd, int16_t speedM);
double speed_PI_control(double dt, double input, double setpoint, double Kp, double Ki);
double stability_PD_control(double dt, double input, double setpoint, double Kp, double Kd);

#endif
