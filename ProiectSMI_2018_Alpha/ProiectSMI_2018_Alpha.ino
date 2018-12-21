#include <SD_t3.h>
#include <SD.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <IRremote.h>


#define LED 13   // Pini Control Instalatie
#define inst1 5
#define inst2 4
#define inst3 3
#define inst4 2
#define RECV_PIN 32
#define MIC_PIN 31

IRrecv irrecv(RECV_PIN);
decode_results results;


// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=69,142
AudioPlaySdWav           playSdWav2;     //xy=69,312
AudioAmplifier           amp3;           //xy=226,115
AudioAmplifier           amp4;           //xy=228,177
AudioAmplifier           amp6;           //xy=228,351
AudioAmplifier           amp5;           //xy=230,281
AudioMixer4              mixer1;         //xy=400,171
AudioMixer4              mixer2;         //xy=402,323
AudioAnalyzePeak         peak1;          //xy=557,148
AudioAmplifier           amp1;           //xy=557,206
AudioAmplifier           amp2;           //xy=559,292
AudioAnalyzePeak         peak2;          //xy=560,350
AudioOutputAnalogStereo  dacs1;          //xy=728,246
AudioConnection          patchCord1(playSdWav1, 0, amp3, 0);
AudioConnection          patchCord2(playSdWav1, 1, amp4, 0);
AudioConnection          patchCord3(playSdWav2, 0, amp5, 0);
AudioConnection          patchCord4(playSdWav2, 1, amp6, 0);
AudioConnection          patchCord5(amp3, 0, mixer1, 0);
AudioConnection          patchCord6(amp4, 0, mixer2, 0);
AudioConnection          patchCord7(amp6, 0, mixer2, 1);
AudioConnection          patchCord8(amp5, 0, mixer1, 1);
AudioConnection          patchCord9(mixer1, peak1);
AudioConnection          patchCord10(mixer1, amp1);
AudioConnection          patchCord11(mixer2, amp2);
AudioConnection          patchCord12(mixer2, peak2);
AudioConnection          patchCord13(amp1, 0, dacs1, 0);
AudioConnection          patchCord14(amp2, 0, dacs1, 1);
// GUItool: end automatically generated code


volatile int state = 0;
volatile float vol = 0.5;
volatile float WavTransition_counter = 1;
volatile int joc = 1;
String track_names[7] = {"SDTEST1.WAV","SDTEST2.WAV","SDTEST3.WAV","SDTEST4.WAV","SDTEST5.WAV"};
volatile int current_track = 1;
volatile int next_track = 1;

bool PlayDevice = 0;

float vol_copy;
int state_copy = state;
float left;
float right;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  AudioMemory(160);
  if(!(SD.begin(BUILTIN_SDCARD))){
    Serial.println("Unable to access the SD card");
    while(1){ }
  }
  pinMode(LED, OUTPUT);
  pinMode(inst1, OUTPUT);
  pinMode(inst2, OUTPUT);
  pinMode(inst3, OUTPUT);
  pinMode(inst4, OUTPUT);
  digitalWrite(inst1, HIGH);
  digitalWrite(inst2, HIGH);
  digitalWrite(inst3, HIGH);
  digitalWrite(inst4, HIGH);

  
  amp1.gain(vol);
  amp2.gain(vol);
  amp3.gain(1);
  amp4.gain(1);
  amp5.gain(0);
  amp6.gain(0);
  mixer1.gain(0,0.8);
  mixer1.gain(1,0.8);
  mixer2.gain(0,0.8);
  mixer2.gain(1,0.8);
  irrecv.enableIRIn(); // Start the receiver
  Timer1.initialize(140000);    //Timer every 0.15 seconds
  Timer1.attachInterrupt(IRInterrupt);    // Attaching Interrupter for timer period.
  Timer3.initialize(300000);    //Timer every 0.10 seconds  for music transition.
  
}

// MIC variables

elapsedMillis msecs;
const int sampleWindow = 100; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;


double volts;       
double last_volts = 0;
unsigned long startMillis= millis();  // Start of sample window
unsigned int peakToPeak = 0;   // peak-to-peak level
 
unsigned int signalMax = 0;
unsigned int signalMin = 1024;

//Defines Joc Lumini 1.
int tree1 = 2;
int tree2 = 2;
int tree3 = 3;
int tree4 = 3;
int tree5 = 4;
int tree6 = 4;
int tree7 = 5;
int tree8 = 5;
int s = 150;
int ds = 225;
int e = 300;
int de = 450;
int q = 600;
int dq = 900;
int h = 1200;
int dh = 1800;
int pPin = 6;


void loop() {                   //  Main  Loop with the State Machine
  // put your main code here, to run repeatedly: 
     switch(state){
          case(0):{ 
            SleepDevice();   // 0  -  Oprit, 1  -  Joc lumini hardcoded  ,  2  -  Redare fisiere audio cu joc lumini generat random ,  3  -  joc lumini dupa microfon
          break;}
          case(1):{
            JocLumini();
            break;
          }
          case(2):{
            PlayAudio();
            break;
          }
          case(3):{
            JocMicrofon();
            break;
          }
        }
  }


void IRInterrupt(){
  if (irrecv.decode(&results)) {     // Infrared Protocol implemented as fixed time Interrupt.
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  switch(results.value){
    case(0xE0E040BF):{ // ON/OFF
      if(state){
        state = 0;
      }
      else{
        state = 1;
        joc = 1;
      }
      break;
    }
    case(0xE0E0F00F):{ // MUTE
      if(vol > 0){
        vol_copy = vol;
        vol = 0;
      }
      else{
        vol  = vol_copy;
      }
      amp1.gain(vol);
      amp2.gain(vol);
      break;
    }
    case(0xE0E058A7):{ // MENU
      if(state){
        state = (state%3) + 1;
        joc = 1;
        current_track = 1;
        next_track = 1;      
      }
      break;
    }
    case(0xE0E020DF):{ // 1
      switch(state){
        case(1):{
             joc = 1;
           break;}
        case(2):{
          if(current_track != 1){
            next_track = 1;
            }
            break;}
        }
      break;
    }
    case(0xE0E0A05F):{ // 2
      switch(state){
        case(1):{
             joc = 2;
           break;}
        case(2):{
          if(current_track != 2){
            next_track = 2;
            }
            break;}
        }
        break;}
    case(0xE0E0609F):{ // 3
      switch(state){
        case(1):{
             joc = 3;
           break;}
        case(2):{
          if(current_track != 3){
            next_track = 3;
            }
            break;}
        }
      break;
    }
    case(0xE0E010EF):{ // 4
      switch(state){
        case(1):{
          joc  = 4;
        }
        case(2):{
          if(current_track != 4){
            next_track = 4;
            }
            break;}
        }
      break;
    }
    case(0xE0E0906F):{ // 5
      switch(state){
        case(2):{
          if(current_track != 5){
            next_track = 5;
            }
            break;}
        }
      break;
    }
    case(0xE0E0E01F):{ // VOL+
      if(vol < 0.5) vol += 0.05;
      amp1.gain(vol);
      amp2.gain(vol);
      break;
    }
    case(0xE0E0D02F):{ // VOL-
      if(vol > 0.1) vol -= 0.05;
      amp1.gain(vol);
      amp2.gain(vol);
      break;
    }
  }
    irrecv.resume(); // Receive the next value
  }
}

void WavTransition(){                    //   Protocol Music  Transition 
  noInterrupts();
  if(WavTransition_counter <10){
    WavTransition_counter += 1;
    if(PlayDevice){
      amp3.gain(1 - WavTransition_counter/10);
      amp5.gain(0 + WavTransition_counter/10);
      amp4.gain(1 - WavTransition_counter/10);
      amp6.gain(0 + WavTransition_counter/10);
    }else{
      amp3.gain(0 + WavTransition_counter/10);
      amp5.gain(1 - WavTransition_counter/10);
      amp4.gain(0 + WavTransition_counter/10);
      amp6.gain(1 - WavTransition_counter/10);
    }
  }else{
    Serial.println("Transition end");
    Timer3.detachInterrupt();
    WavTransition_counter = 1;
     if(!PlayDevice){
      playSdWav2.stop();
    }else{
      playSdWav1.stop();
    }
  }
  interrupts();
}

void PlayAudio(){                      // Protocol  Audio Interface
        digitalWrite(inst1, HIGH);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
    if(state == 2){
      noInterrupts();
      PlayDevice = !PlayDevice;
     interrupts();
    }
  if(PlayDevice){
         amp3.gain(1);
         amp4.gain(1);
         amp5.gain(0);
         amp6.gain(0);
       playSdWav2.play(track_names[current_track - 1].c_str());
       delay(20);
       WavTransition_counter = 1;
       Timer3.attachInterrupt(WavTransition);
       while(playSdWav2.isPlaying()&& state == 2 && current_track == next_track){
        if(msecs > 800){
          msecs = 0;
          if(peak1.available() && peak2.available()){
            left = peak1.read();
            right = peak2.read();
            digitalWrite(inst1, HIGH);
            digitalWrite(inst2, HIGH);
            digitalWrite(inst3, HIGH);
            digitalWrite(inst4, HIGH);
            if(left > 0.15){
              digitalWrite(inst1, LOW);
            }
            if(left*random(0,3) > 0.20){
              digitalWrite(inst2, LOW);
            }
            if(right > 0.15){
              digitalWrite(inst3, LOW);
            }
            if(right*random(0,3) > 0.20){
              digitalWrite(inst4, LOW);
            }
          }
        }
       }

  }else{
       amp3.gain(0);
       amp4.gain(0);
       amp5.gain(1);
       amp6.gain(1);
       playSdWav1.play(track_names[current_track - 1].c_str());
       delay(500);
       WavTransition_counter = 1;
       Timer3.attachInterrupt(WavTransition);
       while(playSdWav1.isPlaying() && state == 2 && current_track == next_track){
          if(msecs > 800){
          msecs = 0;
          if(peak1.available() && peak2.available()){
            left = peak1.read();
            right = peak2.read();
            digitalWrite(inst1, HIGH);
            digitalWrite(inst2, HIGH);
            digitalWrite(inst3, HIGH);
            digitalWrite(inst4, HIGH);
            if(left > 0.15){
              digitalWrite(inst1, LOW);
            }
            if(left*random(0,3) > 0.20){
              digitalWrite(inst2, LOW);
            }
            if(right > 0.15){
              digitalWrite(inst3, LOW);
            }
            if(right*random(0,3) > 0.20){
              digitalWrite(inst4, LOW);
            }
          }
        }
       }    
    }
    if(state == 2){
      noInterrupts();
    if(current_track == next_track){
        current_track = (current_track + 1)%5;
        next_track = current_track;
      }
    else{current_track = next_track;}
      interrupts();
    }
}

void SleepDevice(){  // Protocol Sleep Mode
  digitalWrite(inst1, HIGH);
  digitalWrite(inst2, HIGH);
  digitalWrite(inst3, HIGH);
  digitalWrite(inst4, HIGH);
  playSdWav1.stop();
  playSdWav2.stop();
  while(state == 0){
    Serial.println("Device Sleeping");
    delay(100);
  }
}

void JocLumini(){    // Protocol Light Show
 digitalWrite(inst1, HIGH);
 digitalWrite(inst2, HIGH);
 digitalWrite(inst3, HIGH);
 digitalWrite(inst4, HIGH);
 playSdWav1.stop();
 playSdWav2.stop();
 switch(joc){
    case(1):{
  while(joc == 1 && state == 1){
  // event 1
  noTone(pPin);
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 261.63); // c4
  delay(q);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 2
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 246.94);  // b3
  delay(de);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 3
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 220); // a3
  delay(s);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 4
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 196); // g3
  delay(1050);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 5
  if(joc != 1 || state != 1) break;
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 174.61); //f3
  delay(s);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 6
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(q);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 7
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 146.83); // d3
  delay(q);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 8
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 130.81); //c3
  delay(dq);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 9
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 196); // g3
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 10
  if(joc != 1 || state != 1) break;
  digitalWrite(tree2, LOW);
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 220); // a3
  delay(dq);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  digitalWrite(tree7, HIGH);
  // event 11
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 220);  // a3
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 12
  digitalWrite(tree3, LOW);
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 246.94); //b3
  delay(dq);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  digitalWrite(tree6, HIGH);
  // event 13
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 246.94);  //b3
  delay(e);
  noTone(pPin);
  if(joc != 1 || state != 1) break;
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 14
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree3, LOW);
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  digitalWrite(tree6, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 261.63); //c4
  delay(dq);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  digitalWrite(tree6, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 15
  digitalWrite(tree8, LOW);
  tone(pPin, 261.63); //c4
  delay(e);
  noTone(pPin);
  digitalWrite(tree8, HIGH);
  // event 16
  digitalWrite(tree1, LOW);
  tone(pPin, 261.63); //c4
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  // event 17
  digitalWrite(tree7, LOW);
  tone(pPin, 246.94); //b3
  delay(e);
  noTone(pPin);
  digitalWrite(tree7, HIGH);
  // event 18
  digitalWrite(tree2, LOW);
  tone(pPin, 220); //a3
  delay(e);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  // event 19
  digitalWrite(tree6, LOW);
  tone(pPin, 196); //g3
  delay(e);
  noTone(pPin);
  digitalWrite(tree6, HIGH);
  // event 20
  digitalWrite(tree3, LOW);
  tone(pPin, 196); //g3
  delay(de);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  // event 21
  if(joc != 1 || state != 1) break;
  digitalWrite(tree5, LOW);
  tone(pPin, 174.61); //f3
  delay(s);
  noTone(pPin);
  digitalWrite(tree5, HIGH);
  // event 22
  digitalWrite(tree4, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  // event 23
  digitalWrite(tree8, LOW);
  tone(pPin, 261.63); //c4
  delay(e);
  noTone(pPin);
  digitalWrite(tree8, HIGH);
  // event 24
  digitalWrite(tree1, LOW);
  tone(pPin, 261.63); //c4
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  // event 25
  digitalWrite(tree7, LOW);
  tone(pPin, 246.94); //b3
  delay(e);
  noTone(pPin);
  digitalWrite(tree7, HIGH);
  // event 26
  if(joc != 1 || state != 1) break;
  digitalWrite(tree2, LOW);
  tone(pPin, 220); //a3
  delay(e);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  // event 27
  digitalWrite(tree6, LOW);
  tone(pPin, 196); //g3
  delay(e);
  noTone(pPin);
  digitalWrite(tree6, HIGH);
  // event 27
  digitalWrite(tree3, LOW);
  tone(pPin, 196); //g3
  delay(de);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  // event 29
  digitalWrite(tree5, LOW);
  tone(pPin, 174.61); //f3
  delay(s);
  noTone(pPin);
  digitalWrite(tree5, HIGH);
  // event 30
  digitalWrite(tree4, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  // event 31
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 32
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 33
  if(joc != 1 || state != 1) break;
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 34
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 35
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(75);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 36
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 174.61); //f3
  delay(75);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 37
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 196); //g3
  delay(dq);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 38
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 174.61); //f3
  delay(75);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 39
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 164.81); //e3
  delay(75);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 40
  if(joc != 1 || state != 1) break;
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 146.83); //d3
  delay(e);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 41
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 146.83); //d3
  delay(e);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 42
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 146.83); //d3
  delay(e);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 43
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 146.83); //d3
  delay(75);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 44
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(75);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 45
  if(joc != 1 || state != 1) break;
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 174.61); //f3
  delay(dq);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 46
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 164.81); //e3
  delay(75);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 47
  digitalWrite(tree2, LOW);
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 146.83); //d3
  delay(75);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  digitalWrite(tree7, HIGH);
  // event 48
  digitalWrite(tree3, LOW);
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 130.81); //c3
  delay(e);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  digitalWrite(tree6, HIGH);
  // event 49
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree3, LOW);
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  digitalWrite(tree6, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 261.63); //c4
  delay(q);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  digitalWrite(tree6, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  // event 50
  if(joc != 1 || state != 1) break;
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 220); //a3
  delay(e);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 51
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 196); //g3
  delay(de);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 52
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 174.61); //f3
  delay(s);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 53
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  tone(pPin, 164.81); //e3
  delay(e);
  noTone(pPin);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  // event 54
  digitalWrite(tree3, LOW);
  digitalWrite(tree6, LOW);
  tone(pPin, 174.61); //f3
  delay(e);
  noTone(pPin);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree6, HIGH);
  // event 55
  if(joc != 1 || state != 1) break;
  digitalWrite(tree2, LOW);
  digitalWrite(tree7, LOW);
  tone(pPin, 164.81); //e3
  delay(q);
  noTone(pPin);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree7, HIGH);
  // event 56
  digitalWrite(tree1, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 146.83); //d3
  delay(q);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree8, HIGH);
  // event 57
  digitalWrite(tree1, LOW);
  digitalWrite(tree2, LOW);
  digitalWrite(tree3, LOW);
  digitalWrite(tree4, LOW);
  digitalWrite(tree5, LOW);
  digitalWrite(tree6, LOW);
  digitalWrite(tree7, LOW);
  digitalWrite(tree8, LOW);
  tone(pPin, 130.81); //c3
  delay(h);
  noTone(pPin);
  digitalWrite(tree1, HIGH);
  digitalWrite(tree2, HIGH);
  digitalWrite(tree3, HIGH);
  digitalWrite(tree4, HIGH);
  digitalWrite(tree5, HIGH);
  digitalWrite(tree6, HIGH);
  digitalWrite(tree7, HIGH);
  digitalWrite(tree8, HIGH);
  delay(500);
        }     break;}
    case(2):{
      while(joc == 2 && state == 1){
      int fib_vec[4] = {1,1,2,3};
      while(fib_vec[3] < 256){
        digitalWrite(inst1, HIGH);
        digitalWrite(inst1, LOW);
        delay(300 - fib_vec[0]);
        digitalWrite(inst1, HIGH);
        digitalWrite(inst2, LOW);
        delay(300 - fib_vec[1] + fib_vec[0]);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, LOW);
        delay(300 - fib_vec[2] + fib_vec[1]);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, LOW);
        delay(300 - fib_vec[3] + fib_vec[2]);
        if(joc != 2 || state != 1) break;
        fib_vec[0] = fib_vec[1];
        fib_vec[1] = fib_vec[2];
        fib_vec[2] = fib_vec[3];
        fib_vec[3] = fib_vec[1] + fib_vec[0];
      }
        digitalWrite(inst1, HIGH);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
      }    break;}
    case(3):{
      while(joc == 3 && state == 1){
    for(int i = 0; i < 15; i++){
      if(joc != 3) break;
    digitalWrite(inst4, LOW);
    delay(200);
    digitalWrite(inst4, HIGH);
    delay(200);
    digitalWrite(inst1, LOW);
    digitalWrite(inst4, LOW);
    delay(200);
    digitalWrite(inst4, HIGH);
    delay(200);
    digitalWrite(inst1, HIGH);}

    
    delay(100);
    digitalWrite(inst1, HIGH);
    digitalWrite(inst2, HIGH);
    digitalWrite(inst3, HIGH);
    digitalWrite(inst4, HIGH);
    for(int i = 0; i < 4; i++){
      if(joc != 3) break;
      digitalWrite(inst1, HIGH);
      digitalWrite(inst2, HIGH);
      digitalWrite(inst3, HIGH);
      delay(100);
      digitalWrite(inst4, HIGH);
      delay(100);
      digitalWrite(inst3, LOW);
      digitalWrite(inst2, LOW);
      digitalWrite(inst1, LOW);
      delay(100);
      digitalWrite(inst4, LOW);
      delay(100);
    }
    
    for(int i = 0; i < 10; i++){
      if(joc != 3) break;
      digitalWrite(inst2, HIGH);
      digitalWrite(inst3, HIGH);
      digitalWrite(inst4, LOW);
      delay(200);
      digitalWrite(inst4, HIGH);
      delay(200);
      digitalWrite(inst1, LOW);
      digitalWrite(inst4, LOW);
      delay(200);
      digitalWrite(inst3, LOW);
      digitalWrite(inst4, HIGH);
      delay(200);
      digitalWrite(inst1, HIGH);
      digitalWrite(inst2,LOW);
    }
    for(int i = 0; i < 4; i++){
      if(joc != 3) break;
      digitalWrite(inst1, HIGH);
      digitalWrite(inst2, HIGH);
      digitalWrite(inst3, HIGH);
      delay(100);
      digitalWrite(inst4, HIGH);
      delay(100);
      digitalWrite(inst3, LOW);
      digitalWrite(inst2, LOW);
      digitalWrite(inst1, LOW);
      delay(100);
      digitalWrite(inst4, LOW);
      delay(100);
    }
    
  }
      break;}
    case(4):{
      int active_light = 0;
      int light_vect[4] = {inst1 , inst2, inst3 ,inst4};
      while(joc == 4 && state == 1){
        digitalWrite(light_vect[active_light],LOW);
        delay(1000);
        for(int i = 0; i < 4 && joc == 4 && state == 1; i++)
        {
          if(i % 4 != active_light && active_light <= i % 4 ){
            digitalWrite(light_vect[i % 4],LOW);
            delay(600);
            digitalWrite(light_vect[i % 4],HIGH);
          }
        }
        digitalWrite(light_vect[active_light],HIGH);
        active_light += 1;
        active_light = active_light % 4;
      }
    }
 }
}

void JocMicrofon(){     //   Protocol MIC Sound Bar
        digitalWrite(inst1, HIGH);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
        playSdWav1.stop();
        playSdWav2.stop();
        float volcp = 0.5;
  while(state == 3){
    startMillis= millis();
     peakToPeak = 0;
     signalMax = 0;
     signalMin = 1024;
   // collect data for 100 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(MIC_PIN);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   volcp = vol;
   Serial.println(volcp);
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   peakToPeak = peakToPeak * pow((2 - volcp),4);
   volts = (peakToPeak * 5.0) / 1024;  // convert to volts
   
   switch((int)volts){
    case(0):{
        last_volts = volts;
        digitalWrite(inst1, HIGH);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
      break;
    }
    case(1):{
        digitalWrite(inst1, LOW);
        digitalWrite(inst2, HIGH);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
        last_volts = volts;
      break;
    }
    case(2):{
        digitalWrite(inst1, LOW);
        digitalWrite(inst2, LOW);
        digitalWrite(inst3, HIGH);
        digitalWrite(inst4, HIGH);
        last_volts = volts;
      break;
    }
    case(3):{
        digitalWrite(inst1, LOW);
        digitalWrite(inst2, LOW);
        digitalWrite(inst3, LOW);
        digitalWrite(inst4, HIGH);
        last_volts = volts;
      break;
    }
    case(4):{
        digitalWrite(inst1, LOW);
        digitalWrite(inst2, LOW);
        digitalWrite(inst3, LOW);
        digitalWrite(inst4, LOW);
        last_volts = volts;
      break;
    }
   }
  }
}
