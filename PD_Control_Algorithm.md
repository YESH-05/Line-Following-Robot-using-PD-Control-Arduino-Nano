# PD Control Algorithm

The robot uses Proportional-Derivative (PD) control.

1. The QTR-8A library estimates the black-line position from 0 to 7000.
2. The target position is 3500, the center of the eight-sensor array.
3. Error is calculated as `position - targetPosition`.
4. The proportional term is `Kp * error`.
5. The derivative term is `Kd * (error - previousError)`.
6. The combined correction is added to the left motor command and subtracted from the right motor command.

`correction = Kp * error + Kd * derivative`

The derivative term responds to changes in error. There is no integral term, so this controller is PD, not PID.

Supplied values: Kp = 0.095, Kd = 1.35, base speed = 130, maximum speed = 180, target = 3500.

The code also remembers the last significant line direction and uses it during line-loss or edge recovery.
