

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

#include <Arduino.h>
#include <MIDI.h>

//-- Constants
#define N_CHANNELS 8
static const uint8_t input_pins[N_CHANNELS] = {A2, A3, A4, A5, A9, A8, A7, A6};
static const uint8_t channel_detect[N_CHANNELS] = {9, 8, 7, 6, 2, 3, 4, 5};
#define BAUD_RATE 9600


void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Shiva by UC3Music");

}

void loop()
{
  //-- Variables
  static uint16_t sensor_values[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  static uint16_t trigger_thresholds[N_CHANNELS] = {100, 100, 100, 100, 100, 100, 100, 100};
  static uint16_t off_thresholds[N_CHANNELS] = {50, 50, 50, 50, 50, 50, 50, 50};
  static uint8_t states[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t channel_enabled[N_CHANNELS] = {0, 1, 0, 0, 0, 0, 0};

  //-- Read analog inputs for active channels
  for (uint8_t i = 0; i < N_CHANNELS; i++)
    if (channel_enabled[i])
      sensor_values[i] = analogRead(input_pins[i]);

  //-- Manage state transitions for all channels
  for (uint8_t i = 0; i < N_CHANNELS; i++)
  {
    if (states[i] == 0 && sensor_values[i] > trigger_thresholds[i])
    {
      //-- Transition to state "ON" and send on note
      states[i] = 1;
      usbMIDI.sendNoteOn(49, 64, 9);   // middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
    else if (states[i] == 1 && sensor_values[i] < off_thresholds[i])
    {
      //-- Transition to state "OFF" and send off note
      states[i] = 0;
      usbMIDI.sendNoteOff(49, 64, 9);  //  middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
  }

  //-- MIDI Controllers should discard incoming MIDI messages
  while (usbMIDI.read()) {
  }

  //-- Short delay to let ADC settle after the last reading
  delay(2);
}
