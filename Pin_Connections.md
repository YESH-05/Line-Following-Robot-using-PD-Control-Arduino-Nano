# Hardware Pin Connections

Pin mapping transcribed from the supplied sketch.

| Module signal | Arduino Nano pin |
|---|---|
| L298N ENA | D5 |
| L298N IN1 | D3 |
| L298N IN2 | D4 |
| L298N IN3 | D7 |
| L298N IN4 | D8 |
| L298N ENB | D6 |
| Action button | D9 |
| Sensor-power control output | D10 |
| QTR-8A outputs | A0, A1, A2, A3, A4, A5, A6, A7 |

Verify motor polarity, sensor order and the D10 sensor-power arrangement against the physical robot. Use a suitable motor supply and common ground. Do not power motors directly from Arduino GPIO pins.
