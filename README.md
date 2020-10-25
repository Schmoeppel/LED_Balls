# LED_Balls
Project for programmable LED-Juggling-Balls with an acc-sensor.  
  
Idea:  
Juggling balls contain an arduino, 3 RGB-LEDs, a Bluetooth-chip and a acc-sensor  
Via an App the Balls can be controlled. Currently those things are possible:  
+choose the balls color  
+set the balls to only light up when they are in the air (acc-sensor is used)  
+run a fixed sequence of lights simultaneously with a song  
  
note: I tried to make the balls recognize the sideswap-pattern they are trown in. But the sensor turned out to be too inaccurate for mentioned task  
  
  
Explaination of all files:  
  
Arduino_LED_Ball_Program.ino:  
File which contains the Program that runs the Arduino. It's not very clean yet...  
  
Ball_inner_half/outer_half.stl:  
contain the 3D-Objects to print the juggling Ball. Its two halfes, you can easily take apart and put back together in just a second. For printing use these settings:  
Material: TPU (harder Materials won't work)  
layerheight: 0.1mm  
support: touching bed  
infill: outer_half -> no infill; inner_half -> 10% infill  
  
LED_Ball_Beschl_Sensor_6.apk:  
Code for the phone-app to control up to 6 balls. Created with MIT-Appinventor.  
I am currently learning Flutter to program I nicer and more userfriendly app, which contains more features.  
  
Wiring.fzz:  
Contains the circuit diagram for the hardware components. Created in Fritzing. For easy acces I also included the same diagram as wiring.png  
Components are:  
-Arduino Nano  
-Bluetooth-Chip: HC-05  
-Acc-sensor: MPU6050  
-3x RGB-LEDs with common ground  
-2x 4.7k-pull-up-resistors (normaly the MPU doesn't need them. But for me it sometimes crashed every couple minutes if I wasn't using them)  
-9V Battery  
  
note: probably the esp32 is a better choice as a microcontroller since it has Bluetooth build in, is faster and draws less current. But I didn't know about it until I finished my juggling balls... :(  
