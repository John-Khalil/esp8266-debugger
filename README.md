# esp8266-debugger

Hi there, im John Khalil

I have built this project to make a simple and powerfull debugging tool that is bassed on the ESP8266 to make it extremly affordable for any one.

This tool include a quad channel logic analyzer that can catch pulses up to 2us in width and a quad channel serial port that support UART in both read and write modes and SPI in read only mode that can be clocked upto 2Mbps (yes you can do console logs @2Mbps from any other Microcontroller in SPI mode only), however there is future plans to add a single channel oscilloscope.

(Note: this project Built using the nonos sdk version 2.7.4 and the ESP8266 was clocked at 80MHz)

here are some pics to demonstrate:

Here is an example of the logic Analyzer sampling the signal of HCSR-04 ultrasonic sensor
<p align="left">
  <img src="https://github.com/John-Khalil/esp8266-debugger/blob/main/pics/hcsr04%20test.jpg?raw=true" width="480" >
</p>

PWM signal @ 1KHz
<p align="left">
  <img src="https://github.com/John-Khalil/esp8266-debugger/blob/main/pics/pwm%20test.jpg?raw=true" width="480" >
</p>

SPI CLK and DATA line
<p align="left">
  <img src="https://github.com/John-Khalil/esp8266-debugger/blob/main/pics/SPI%20test.jpg?raw=true" width="480" >
</p>

SERIAL PORT for console logs
<p align="left">
  <img src="https://github.com/John-Khalil/esp8266-debugger/blob/main/pics/spi%20console1.jpg?raw=true" width="480" >
</p>
