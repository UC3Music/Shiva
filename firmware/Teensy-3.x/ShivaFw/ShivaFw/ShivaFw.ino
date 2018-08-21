

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
#include <Bounce.h>

//-- Constants
#define N_CHANNELS 8
#define BLINKING 2
#define BLINKING_RATE 2
#define SENSOR_DETECT_RATE 500 //-- in ms
#define JOYSTICK_READ_RATE 20 //-- in ms
#define BAUD_RATE 9600
static const uint8_t input_pins[N_CHANNELS] = {A2, A3, A4, A5, A9, A8, A7, A6};
static const uint8_t channel_detect[N_CHANNELS] = {9, 8, 7, 6, 2, 3, 4, 5};
static const uint8_t common_notes[19] = {36, //-- Bass Drum 1
                                         35, //-- Bass Drum 2
                                         38, //-- Snare Drum 1
                                         40, //-- Snare Drum 2
                                         39, //-- Hand Clap
                                         43, //-- Low Tom 1
                                         41, //-- Low Tom 2
                                         47, //-- Mid Tom 1
                                         45, //-- Mid Tom 2
                                         50, //-- High Tom 1
                                         48, //-- High Tom 2
                                         46, //-- Open Hi-hat
                                         42, //-- Closed Hi-hat
                                         44, //-- Pedal Hi-hat
                                         49, //-- Crash Cymbal 1
                                         57, //-- Crash Cymbal 2
                                         51, //-- Ride Cymbal 1
                                         58, //-- Ride Cymbal 2
                                         52  //-- Chinese Cymbal
                                         }; //-- These are used to select the notes using the joystick

//-- Serial stuff
String buffer = "";
bool incoming_cmd = false;

//-- Global variables
static uint8_t verbose = false;
static uint8_t channel_enabled[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t led_status[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t notes[N_CHANNELS] = {49, 49, 49, 49, 49, 49, 49, 49};

//-- Counting time
static elapsedMillis since_sensor_detect;
static elapsedMillis since_joystick_read;

//-- Joystick-related
static uint8_t button_pins[5] = {A0, 10, 12, A1, 11}; //-- up, down, left, right, enter
static Bounce buttons[5] = { Bounce(button_pins[0], 10), //-- 10ms debounce
                             Bounce(button_pins[1], 10),
                             Bounce(button_pins[2], 10),
                             Bounce(button_pins[3], 10),
                             Bounce(button_pins[4], 10)};
static uint8_t button_status[5] = {0, 0, 0, 0, 0};

//-- Detection thresholds
static uint16_t trigger_thresholds[N_CHANNELS] = {100, 100, 100, 100, 100, 100, 100, 100};
static uint16_t off_thresholds[N_CHANNELS] = {50, 50, 50, 50, 50, 50, 50, 50};


void update_leds() 
{
  for (int i = 0; i < N_CHANNELS; i++)
  {
    //if (channel_enabled[i])  //-- LEDs will only turn on if the piezo sensor is present (hw constraints)
    {
      pinMode(channel_detect[i], OUTPUT);
      if (led_status[i] == BLINKING)
        tone(channel_detect[i], BLINKING_RATE);
      else
        digitalWrite(channel_detect[i], led_status[i]==1);
    }
   }
}

void read_channel_status()
{
  for (int i = 0; i < N_CHANNELS; i++)
  {
    noTone(i); //-- Tone interferes with status detection
    pinMode(channel_detect[i], INPUT_PULLUP);
    channel_enabled[i]=digitalRead(channel_detect[i]);
  }
}

void parse_serial_command()
{
    //-- Available commands:
    //-- S0 -> Enable verbose mode
    //-- S1 -> Disable verbose mode
    //-- S2CxVy -> Edit note y in channel x
    //-- S3CxVy -> Edit upper threshold (trigger) y in channel x
    //-- S4CxVy -> Edit lower threshold (off) y in channel x
    //-- S5Cx -> Query current note for channel x
    //-- S6Cx -> Query current upper threshold (trigger) y in channel x
    //-- S7Cx -> Query current lower threshold (off) y in channel x
    //-- S8Cx -> Query channel status (enabled/disabled)
    
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
      else if (buffer[1]=='5')
      {
        //-- Query current note for channel x
        uint8_t channel = buffer.substring(3).toInt();
        if (channel < N_CHANNELS)
          Serial.println("S5C"+String(channel)+"V"+String(notes[channel]));
      }
      else if (buffer[1]=='6')
      {
        //-- Query current upper threshold (trigger) for channel x
        uint8_t channel = buffer.substring(3).toInt();
        if (channel < N_CHANNELS)
          Serial.println("S6C"+String(channel)+"V"+String(trigger_thresholds[channel]));
      }
      else if (buffer[1]=='7')
      {
        //-- Query current upper threshold (trigger) for channel x
        uint8_t channel = buffer.substring(3).toInt();
        if (channel < N_CHANNELS)
          Serial.println("S7C"+String(channel)+"V"+String(off_thresholds[channel]));
      }
      else if (buffer[1]=='8')
      {
        //-- Query status (enabled/disabled) for channel x
        uint8_t channel = buffer.substring(3).toInt();
        if (channel < N_CHANNELS)
          Serial.println("S8C"+String(channel)+"V"+String(channel_enabled[channel]));
      }
    }

    buffer = "";
}

void setup() 
{
  //-- Init joystick
  for (int i = 0; i < 5; i++)
  {
    pinMode(button_pins[i], INPUT_PULLUP);
  }

  //-- Init serial port
  Serial.begin(BAUD_RATE);
  Serial.println("Shiva by UC3Music");

  //-- Flash LEDs at startup
  for (int i = 0; i < N_CHANNELS; i++)
  {
    digitalWrite(channel_detect[i], HIGH);
    delay(200);
  }

  update_leds();
}

void loop()
{
  //-- Variables
  static uint16_t sensor_values[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t states[N_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};

  //-- Detect active channels
  if (since_sensor_detect >= SENSOR_DETECT_RATE)
  {
    since_sensor_detect-=SENSOR_DETECT_RATE;
    read_channel_status();
    update_leds();
  }


  //-- Read joystick
  if (since_joystick_read >= JOYSTICK_READ_RATE)
  {
    since_joystick_read-=JOYSTICK_READ_RATE;
    for (uint8_t i = 0; i < 5; i++)
      if (buttons[i].update())
        if (buttons[i].risingEdge())
          button_status[i] = 1;
  }


  //-- Check status (dummy test for development)
  for (uint8_t i = 0; i < 5; i++)
    if (button_status[i]==1)
    {
      button_status[i]=0;
      for (uint8_t j=0; j < N_CHANNELS; j++)
        led_status[j] = 0;
      led_status[i] = BLINKING;
      update_leds();
    }
    

  //-- Read analog inputs for active channels
  for (uint8_t i = 0; i < N_CHANNELS; i++)
    if (channel_enabled[i])
    {
      sensor_values[i] = analogRead(input_pins[i]);
      if (verbose)
        Serial.println("S0C"+String(i)+"V"+String(sensor_values[i]));
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
  if (incoming_cmd)
  {
    parse_serial_command();
    incoming_cmd = false;
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
