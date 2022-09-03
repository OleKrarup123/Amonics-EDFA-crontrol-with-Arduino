# Amonics-EDFA-crontrol-with-Arduino
Arduino sketch used for controlling the current supplied to an Amonics high gain EDFA (AEDFA-M) using a rotary encoder and displaying status on a 4x20 LCD screen.

LCD&Shield.png: Screen and interface for communicating with Arduino
Enclosure.png: Metal enclosure used for housing EDFA unit, Arduino and Rotary encoder
RotaryEncoder.png: Rotary encoder+board used for controlling supplied current. 
PowerSupply.png: Typical 5V, 1A power supply that could be used for driving the EDFA. Note that it can be connected directly to the 5V DC and DC GND pins (JP1) of the EDFA control board. Using a 5V pin and a GND pin on this board, power can be supplied to the Arduino. Based on input from the rotary encoder, the Arduino changes the voltage of Pin11 (SET-1) on the EDFA board, thereby regulating the current drawn by the EDFA.  


EDFA_control.ino: Sketch for controlling EDFA. Works but could be enhanced to monitor internal temperature.
IDC 14 Pin.pdf: Info about each pin on EDFA circuit board.
Amonics Email: Discussion of the purpose of each pin on EDFA board. NOTE: PIN1 is the one labelled 5V in the bottom of the picture in IDF 14 Pin.pdf.
AEDFA-m_120x100x18_IDC254_R704(1).pdf: EDFA manual
YouTube Link.txt: Link to YouTube video showing finished device in action.

