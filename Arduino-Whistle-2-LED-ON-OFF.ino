#include  "arduinoFFT.h"
#include <math.h>

#define SAMPLES 128             //SAMPLES-pt FFT. Must  be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 4096  //Ts = Based on Nyquist, must be 2 times the highest expected frequency.
#define ARRAYSIZE 12
#define ARRAYNOTES 3
 
arduinoFFT  FFT = arduinoFFT();
 
unsigned int samplingPeriod;
unsigned long microSeconds;
  
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double  vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values
String vNotes[ARRAYNOTES];


String noteStrings[ARRAYSIZE] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
String currentNote = "";
double percentCorrection = -2.18;
double minFrequency = 400.0;
double minAmplitude = 100.0;
double percTollerance = 2.0;
double currentPeak = 0.0;

void  setup() 
{
    Serial.begin(115200); //Baud rate for the Serial Monitor
    samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY)); //Period in microseconds  
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
}
 
void loop() 
{  
    /*Sample SAMPLES times*/
    for(int  i=0; i<SAMPLES; i++)
    {
        microSeconds = micros();    //Returns the  number of microseconds since the Arduino board began running the current script.  
     
        vReal[i] = analogRead(0); //Reads the value from analog pin  0 (A0), quantize it and save it as a real term.
        vImag[i] = 0; //Makes  imaginary term 0 always

        /*remaining wait time between samples if  necessary*/
        while(micros() < (microSeconds + samplingPeriod))
        {
          //do nothing
        }
    }
 
    double amplitude = average(vReal, SAMPLES);

    if (amplitude >= minAmplitude)
    {

      /*Perform FFT on samples*/
      FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal,  vImag, SAMPLES, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

      /*Find peak frequency and print peak*/
      double peak = FFT.MajorPeak(vReal,  SAMPLES, SAMPLING_FREQUENCY);
      double correctPeak = peak + (peak * percentCorrection / 100);
      double diffPeak = abs(correctPeak - currentPeak);
      double minDiffPeak = currentPeak * percTollerance / 100;

      if ((correctPeak >= minFrequency) && (diffPeak > minDiffPeak))
      {
        String note = noteFromPitch(correctPeak);
        if (note != currentNote)
        {
          /*
          Serial.print(note);     //Print out the most  dominant frequency.
          Serial.print(" : ");     //Print out the most  dominant frequency.
          Serial.print(amplitude);
          Serial.print(" > ");     //Print out the most  dominant frequency.
          Serial.println(correctPeak);
          */
          currentNote = note;
          SetArrayNotes(note);
          if (checkNotes1())
          {
            Action1();
          }
          if (checkNotes2())
          {
            Action2();
          }
          currentPeak = correctPeak;
        }
      }
    }
    delay(10);
}

String noteFromPitch(double frequency) {
	return noteStrings[(round(12 * (log(frequency / 440) / log(2))) + 69) % 12];
}

float average (double * array, int len)  // assuming array is int.
{
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++)
    sum += array [i] ;
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

void SetArrayNotes(String note)
{
  vNotes[0] = vNotes[1];
  vNotes[1] = vNotes[2];
  vNotes[2] = note;
}

bool checkNotes1()
{
  return (vNotes[0] == "E" && vNotes[1] == "F#" && vNotes[2] == "G#" );
}

bool checkNotes2()
{
  return (vNotes[0] == "G#" && vNotes[1] == "F#" && vNotes[2] == "E" );
}

void Action1()
{
  Serial.println("ON > Action1");  
  digitalWrite(6, HIGH);
  delay(500);
  beepShort(2093);
  delay(1000);
  digitalWrite(6, LOW);
  Serial.println("OFF > Action1");  
}


void Action2()
{
  Serial.println("ON > Action2");  
  digitalWrite(7, HIGH);
  delay(500);
  beepLong(2093);
  delay(1000);
  digitalWrite(7, LOW);
  Serial.println("OFF > Action2");  
}

void beepShort(unsigned int freq)
{
  for (int i=0; i<6; i++)
  {
    Serial.println("Buzzer...");  
    tone(8, freq, 200);
    delay(200);
  }
}

void beepLong(unsigned int freq)
{
  for (int i=0; i<6; i++)
  {
    Serial.println("Buzzer...");  
    tone(8, freq, 400);
    delay(400);
  }
}
