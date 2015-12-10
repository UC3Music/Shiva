/*  Shiva
 *  -------------------------------------
 *  Electronic diy midi drumkit
 *  
 *  Author: David Estevez Fernandez (DEF)
 *    https://github.com/David-Estevez
 *
 *  Shiva is a UC3Music project: 
 *    https://github.com/UC3Music/Shiva
 *    
 */
 
 
//-- Hardware pin definitions
static const uint8_t N_CHANNELS = 8;
static const uint8_t CHANNEL_INPUT[N_CHANNELS] = {A2, A3, A4, A5, 
                                                  A9, A8, A7, A6};
static const uint8_t CHANNEL_DETECT[N_CHANNELS] = {9, 8, 7, 6, 2,
                                                   3, 4, 5};

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A2;  // Analog input pin that the potentiometer is attached to

int sensorValue = 0;        // value read from the pot

const int THRESHOLD = 70;
const int THRESHOLD2 = 55;

void setup() {
  Serial.begin(9600);
  Serial.println("Shiva by UC3Music");

}

void loop() {
  static uint8_t state = 0;
  
  // read the analog in value:
  sensorValue = analogRead(analogInPin);

  if (state == 0) {
    if (sensorValue > THRESHOLD) {
      // print the results to the serial monitor:
      Serial.print("sensor = " );
      Serial.println(sensorValue);  

      state = 1;

      // midi note
      usbMIDI.sendNoteOn(49, 64, 9);   // middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
  }
  else if (state == 1) {
    if (sensorValue < THRESHOLD2) {
      state = 0;

      // note off
      usbMIDI.sendNoteOff(49, 64, 9);  //  middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
  }

  // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);
}
