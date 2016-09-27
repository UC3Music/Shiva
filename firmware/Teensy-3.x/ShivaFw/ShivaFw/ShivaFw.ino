

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

//-- Serial stuff
String buffer = "";
bool incoming_cmd = false;

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
  static uint8_t notes[N_CHANNELS] = {49, 49, 49, 49, 49, 49, 49, 49};
  static uint8_t verbose = false;


  //-- Read analog inputs for active channels
  for (uint8_t i = 0; i < N_CHANNELS; i++)
    if (channel_enabled[i])
    {
      sensor_values[i] = analogRead(input_pins[i]);
      if (verbose)
        Serial.println("S"+String(i)+"V"+String(sensor_values[i]));
    }

  //-- Manage state transitions for all channels
  for (uint8_t i = 0; i < N_CHANNELS; i++)
  {
    if (states[i] == 0 && sensor_values[i] > trigger_thresholds[i])
    {
      //-- Transition to state "ON" and send on note
      states[i] = 1;
      usbMIDI.sendNoteOn(notes[i], 64, 9);   // middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
    else if (states[i] == 1 && sensor_values[i] < off_thresholds[i])
    {
      //-- Transition to state "OFF" and send off note
      states[i] = 0;
      usbMIDI.sendNoteOff(notes[i], 64, 9);  //  middle C, normal velocity, channel 9
      usbMIDI.send_now();
    }
  }

  //-- MIDI Controllers should discard incoming MIDI messages
  while (usbMIDI.read()) {
  }

  //-- Take care of serial commands
  //-- Available commands:
  //-- S0 -> Enable verbose mode
  //-- S1 -> Disable verbose mode
  //-- S2CxVy -> Edit note y in channel x
  //-- S3CxVy -> Edit note y in channel x
  //-- S4CxVy -> Edit note y in channel x
  if (incoming_cmd)
  {
    //-- Parse command
    if (buffer[0]=='S')
    {
      if (buffer[1]=='0')
      {
        //-- Enable verbose mode
        verbose = true;
        Serial.println("Ok");
      }
      else if (buffer[1]=='1')
      {
        //-- Disable verbose mode
        verbose = false;
        Serial.println("Ok");
      }
      else if (buffer[1]=='2')
      {
        //-- Edit note per channel
        uint8_t channel_start = buffer.indexOf('C', 1);
        uint8_t value_start = buffer.indexOf('V', channel_start);
        uint8_t channel = buffer.substring(channel_start+1, value_start).toInt();
        uint8_t value = buffer.substring(value_start+1).toInt();
        if (channel < N_CHANNELS)
          notes[channel] = value;
        Serial.println("Ok");
      }
      else if (buffer[1]=='3')
      {
        //-- Edit upper threshold (trigger) per channel
        uint8_t channel_start = buffer.indexOf('C', 1);
        uint8_t value_start = buffer.indexOf('V', channel_start);
        uint8_t channel = buffer.substring(channel_start+1, value_start).toInt();
        uint8_t value = buffer.substring(value_start+1).toInt();
        if (channel < N_CHANNELS)
          trigger_thresholds[channel] = value;
        Serial.println("Ok");
      }
      else if (buffer[1]=='4')
      {
        //-- Edit lower threshold (off) per channel
        uint8_t channel_start = buffer.indexOf('C', 1);
        uint8_t value_start = buffer.indexOf('V', channel_start);
        uint8_t channel = buffer.substring(channel_start+1, value_start).toInt();
        uint8_t value = buffer.substring(value_start+1).toInt();
        if (channel < N_CHANNELS)
          off_thresholds[channel] = value;
        Serial.println("Ok");
      }
    }

    //-- Erase buffer
    incoming_cmd = false;
    buffer = "";

  }
  //-- Short delay to let ADC settle after the last reading
  delay(2);
}


//-- Reads the incoming chars and stores them in the buffer
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    buffer += inChar;
    if (inChar == '\n')
      incoming_cmd = true;
  }
}
