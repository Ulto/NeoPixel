/*******************************
 * CrossFire Neopixels  + Greg *
 ******************************/
 
 /* CUSTOMIZABLE PARAMETERS */
   // Neopixel LED Strip
   #define NumPixels 240                    // number of pixels in strip (255 MAX)
   #define DataOut 8                        // pin number (most are valid)
   #define PixelInfo NEO_GRB + NEO_KHZ800   // pixel type flags, add together as needed:
                                                //   NEO_RGB     Pixels are wired for RGB bitstream
                                                //   NEO_GRB     Pixels are wired for GRB bitstream
                                                //   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
                                                //   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
                                                
   // Power Interlock (Jumper 5V supply pin (Vin) to specified ADC pin.)
   #define PowerInterlock A5              // ADC Pin
   #define PowerThreshold 950             // 0 - 1024
   #define PowerIndicatorBrightness 25    // 0 - 255
   
   // Show Selection / Menu Customization
   #define DefaultMenuSelection 0
   #define SelectionChangeWipeDelay 3
   
   // Random Number Generator Seed (This pin should be left floating.)
   #define FloatingPin A3
   
   // MSQGEQ7 - Pin Numbers Hard Wired on Spectrum Shield
   #define spectrumReset 5
   #define spectrumStrobe 4
   #define spectrumAnalogL A0    // Left Channel
   #define spectrumAnalogR A1    // Right Channel
   
   // Spectrum Function Settings
   #define maxBrightness  15
   
/*************************************************************************************************/
/***||||||||||||||||||||||||| No Editable Parameters Below This Line ||||||||||||||||||||||||||***/
/*************************************************************************************************/
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NumPixels, DataOut, NEO_GRB + NEO_KHZ800);

// Global Variables
char menuSelection = DefaultMenuSelection;
unsigned long pixelSets = 0;

unsigned long previoustime;
unsigned long currentMillis;

char Pulseflag = 1;

void setup() {
  
  // MSGEQ7
  //Setup pins to drive the spectrum analyzer. 
  pinMode(spectrumReset, OUTPUT);
  pinMode(spectrumStrobe, OUTPUT);

  //Init spectrum analyzer
  digitalWrite(spectrumStrobe,LOW);
    delay(1);
  digitalWrite(spectrumReset,HIGH);
    delay(1);
  digitalWrite(spectrumStrobe,HIGH);
    delay(1);
  digitalWrite(spectrumStrobe,LOW);
    delay(1);
  digitalWrite(spectrumReset,LOW);
    delay(5);
  // Reading the analyzer now will read the lowest frequency.
  
  // NeoPixel Initialization
  strip.begin();
  strip.show();    // All pixels OFF
  
  // Random Number Generator Initialization
  randomSeed(analogRead(FloatingPin));    // Use Floating ADC Pin
  
  // Serial Initialization
  Serial.begin(9600);
  
  // Show Type(s) Menu & Selection
  printMenu();
  
}

void loop() {
  powerInterlock();  
    
  switch (menuSelection) {
    case 0:
      // Lights Off
    break;
    
    case 1:
      SetRandomPixel();
    break;
   
    case 2:
      SetRandomPixel();
      KillRandomPixel();
    break;
    
    case 3:
      SetRandomPixelRGB();
    break;
    
    case 4:
      SetRandomPixelRGB();
      KillRandomPixel();
    break;
    
    case 5:
      Spectrum();
    break;
    
    case 6:
      SpectrumBarGraph();
    break;
    
    case 7:
      Spectrum2();
    break;
    
    case 8:
      PulseBlue();
    break;
    
    case 9:
      rainbow(20);
    break;
    
    case 10:
      rainbowCycle(20);
    break;
    
    case 11:
      RandomPixelFade();
    break;
    
  }
  
}

void ReadMSGEQ7(int Spectrum[]) {
    // Band 0 = Lowest Frequencies.
    byte band;
    
    for(band = 0; band < 7; band++) {
      Spectrum[band] = (analogRead(spectrumAnalogL) + analogRead(spectrumAnalogR) + analogRead(spectrumAnalogL) + analogRead(spectrumAnalogR) + analogRead(spectrumAnalogL) + analogRead(spectrumAnalogR)) / 5; // Read several times and take the average
      digitalWrite(spectrumStrobe,HIGH);
      digitalWrite(spectrumStrobe,LOW);     
    }
}

void SpectrumBarGraph() {
  // Spectrum analyzer read values will be kept here.
  static int Spectrum[7];
  char barLength = 40;
  
  // Get current Spectrum data
  ReadMSGEQ7(Spectrum);
    
  barLength = map(Spectrum[0], 0, 255, 0, 40);
  
  for (int i = 50; i <= 90; i++) {
    
    if (barLength > i) {
    strip.setPixelColor(i, 100, 0, 100);
    } else {
      strip.setPixelColor(i, 0, 0, 0);
      
    }
  }
  
  
  
    /*
  for (i = 0; i < 7; i++) {
    barLength = map(Spectrum[i], 0, 255, 0, 20);
    
    
    for (j = 0; j < (currentStartPt + barLength); j++) {
      strip.setPixelColor((currentStartPt + j), 0, 0, 100);
      strip.show();
    }
    
    currentStartPt += 25;
    
    
  }
  
  currentStartPt = 0;
  
  // Clear Srtip
    for(int i = 0; i <= strip.numPixels(); i++) {     
      strip.setPixelColor(i, 0, 0, 0);
      pixelSets++;
    }
  
   pixelSets += 7;
    
    strip.show();
  */
  
} // END SpectrumBarGraph

void Spectrum() {
  // Spectrum analyzer read values will be kept here.
  static int Spectrum[7];
  static char color = 1;  // Spectrum
  
  // Get current Spectrum data
  ReadMSGEQ7(Spectrum);
  
  // Display
  char barLength = 0;
  char i = 0;
  char j = 0;
  char currentStartPt = 0;
  static int SpectrumMAP[7];  // Spectrum values, mapped to desired brightness

  // Strobe Color Rotation Set
  if (Spectrum[0] > 250) {
    color += 1;
    
    if (color > 3) {
      color = 1;
    }
  }
  
  // Repeated Spectrum Strip
  for (int j = 0; j < strip.numPixels(); j += 8) {
    switch (color) {
      case 1:
        strip.setPixelColor((j + 7), 0, 0, 255);
      break;
    
      case 2:
        strip.setPixelColor((j + 7), 0, 255, 0);
      break;
    
      case 3:
        strip.setPixelColor((j + 7), 255, 0, 0);
      break; 
    }
   
    for (int i = 0; i < 7; i++) {
      SpectrumMAP[i] = map(Spectrum[i], 0, 255, 0, maxBrightness);
      strip.setPixelColor((i + j), SpectrumMAP[i], SpectrumMAP[i], SpectrumMAP[i]);
    }
  
  }
  
  pixelSets += strip.numPixels();  // Entire strip has been updated.
  strip.show();
 
}  // END Spectrum

void Spectrum2() {
  // Spectrum analyzer read values will be kept here.
  static int Spectrum[7];
  static char color = 1;
  char averageLevel;
  
  // Get current Spectrum data
  ReadMSGEQ7(Spectrum);
  
  // Display
  char barLength = 0;
  char i = 0;
  char j = 0;
  char currentStartPt = 0;
  static int SpectrumMAP[7];  // Spectrum values, mapped to desired brightness

  averageLevel = (Spectrum[0] + Spectrum[1] + Spectrum[2] + Spectrum[3] + Spectrum[4] + Spectrum[5] + Spectrum[6]) / 7;
  
  // Repeated Spectrum Strip
  for (int j = 0; j < strip.numPixels(); j += 8) {
    switch (color) {
      case 1:
        strip.setPixelColor((j + 7), 0, 0, averageLevel);
      break;
    
      case 2:
        strip.setPixelColor((j + 7), 0, averageLevel, 0);
      break;
    
      case 3:
        strip.setPixelColor((j + 7), averageLevel, 0, 0);
      break; 
    }
   
    for (int i = 0; i < 7; i++) {
      SpectrumMAP[i] = map(Spectrum[i], 0, 255, 0, maxBrightness);
      strip.setPixelColor((i + j), SpectrumMAP[i], SpectrumMAP[i], SpectrumMAP[i]);
    }
  
  }
  
  pixelSets += strip.numPixels();  // Entire strip has been updated.
  strip.show();
 
}  // END Spectrum2

void printMenu() {
  Serial.println("****************************");
  Serial.println(" CrossFire Neopixels + Greg ");
  Serial.println("****************************");
  Serial.println();
  Serial.println(" Please Select a Show Type: ");
  Serial.println(" 0 - Lights Off & Re-Print Menu");
  Serial.println(" 1 - Traditional, Randon pixels set to random vibrant colors. (DEFAULT)");
  Serial.println(" 2 - Traditional + Kill, For every pixel that is set, another is blacked out.");
  Serial.println(" 3 - Traditional RGB Only");
  Serial.println(" 4 - Traditional RGB Only + Kill, For every pixel that is set, another is blacked out.");
  Serial.println(" 5 - Spectrum");
  Serial.println(" 6 - Spectrum Bar Graph");
  Serial.println(" 7 - Spectrum2");
  Serial.println(" 8 - Blue Pulse");
  Serial.println(" 9 - Rainbow");
  Serial.println("10 - Rainbow Cycle"); 
  Serial.println("11 - Random Pixel Fade");
  Serial.println("99 - View Pixel Set Counter (Current Show Setting Retained)");
  Serial.println();
  Serial.println("Show Selection: "); 
}

void powerInterlock() {
  // External Power Interlock Check
  while (analogRead(PowerInterlock) < PowerThreshold) {
    // Blackout Strip
    for(int i = 0; i <= strip.numPixels(); i++) {     
      strip.setPixelColor(i, 0, 0, 0);
      pixelSets++;
    }
    
    strip.setPixelColor(0, PowerIndicatorBrightness, 0, 0);  // Set first pixel dim RED to indicate power issue
    pixelSets++;
    
    strip.show();
    
    Serial.println("NO EXTERNAL POWER DETECTED");
    
    menuSelection = 0;
  }
  
  
}

// Called on presence of data in the HW serial buffer at the end of each main loop execution
void serialEvent() {
   int incomingByte;
   incomingByte = Serial.parseInt();
   
   setShowSelection(incomingByte);
   
   Pulseflag = 1;
}

void setShowSelection(int showSelection) {
  static char menuSelectionTEMP = DefaultMenuSelection;
  
  switch (showSelection) {
     case 0:  // 0 - Lights Off
       menuSelection = 0;
       Serial.println("Option 0 Selected");
       printMenu();
     break;
     
     case 1:  // 1 - Traditional, Randon pixels set to random vibrant colors. (DEFAULT)
       menuSelection = 1;
       Serial.println("Option 1 Selected");
     break;
     
     case 2:  // 2 - Traditional + Kill, For every pixel that is set, another is blacked out.
       menuSelection = 2;
       Serial.println("Option 2 Selected");
     break;
     
     case 3:  // 3 - Traditional RGB Only.
       menuSelection = 3;
       Serial.println("Option 3 Selected");
     break;
     
     case 4:  // 4 - Traditional RGB Only + Kill, For every pixel that is set, another is blacked out.
       menuSelection = 4;
       Serial.println("Option 4 Selected");
     break;
     
     case 5:  // 5 - Spectrum
       menuSelection = 5;
       Serial.println("Option 5 Selected");
     break;
     
     case 6:  // 6 - Spectrum Bar Graph
       menuSelection = 6;
       Serial.println("Option 6 Selected");
     break;
     
     case 7:  // 7 - Spectrum2
       menuSelection = 7;
       Serial.println("Option 7 Selected");
     break;
     
     case 8:  // 8 - BluePulse
       menuSelection = 8;
       Serial.println("Option 8 Selected");
     break;
     
     case 9:  // 9 - Rainbow
       menuSelection = 9;
       Serial.println("Option 9 Selected");
     break;
     
     case 10:  // 10 - Rainbow Cycle
       menuSelection = 10;
       Serial.println("Option 10 Selected");
     break;
     
     case 11:  // 11 - Random Pixel Fade
       menuSelection = 11;
       Serial.println("Option 11 Selected");
     break;
     
     case 99:  // 9 - View Pixel Set Counter (Current Show Setting Retained)
       Serial.print("Pixel Sets Since Start: ");
       Serial.println(pixelSets);
     break;
     
     default:
       Serial.println("INVALID INPUT");
     break;
     
   }
   
  // Only clear strip if show type has changed
  if (menuSelection != menuSelectionTEMP) {
     // Re-Initialize Strip For Show Change / Restart
     EndToEndWipe(SelectionChangeWipeDelay);
     
     menuSelectionTEMP = menuSelection;
  }
   
}

void EndToEndWipe(int updateDelay) {  
   int j = strip.numPixels();
   int i = 1;
   
   // Account for even / odd number of pixels.
   if (j % 2) {
     i = 0;
   } else {
     i = 1;
   }
   
   for(i; i <= (strip.numPixels() + 1); i = i + 2) {
     strip.setPixelColor(i, 0, 0, 0);
     strip.show();  // Show each time for a quick wipe effect.
     delay(updateDelay);
     
     strip.setPixelColor(j, 0, 0, 0);
     strip.show();  // Show each time for a quick wipe effect.
     delay(updateDelay);
     
     pixelSets += 2;
     j -= 2;
   }
}


void KillRandomPixel() {
   strip.setPixelColor(random(0, strip.numPixels()), 0, 0, 0);

  // Push Update to Strip
  strip.show();
  pixelSets++;
}

void SetRandomPixelRGB() {
  // Variable Declarations
  byte rgbAssignment; 
  
  // Determine R-G-B Color Value Assignments
  rgbAssignment = random(1, 4);
  
  switch (rgbAssignment) {
    case 1:
      // Red
      strip.setPixelColor(random(0, strip.numPixels()), 255, 0, 0);
    break;
   
    case 2:
     // Green
     strip.setPixelColor(random(0, strip.numPixels()), 0, 255, 0);
     
    break;
   
    case 3:
     // Blue
     strip.setPixelColor(random(0, strip.numPixels()), 0, 0, 255);
     
    break; 
  }
    
  // Push Update to Strip
  strip.show();
  pixelSets++;
}

void SetRandomPixel() {
  // Variable Declarations
  byte rgbAssignment; 
  
  // Determine R-G-B Color Value Assignments
  rgbAssignment = random(1, 4);
  
  switch (rgbAssignment) {
    case 1:
      // Red-Green Mix
      strip.setPixelColor(random(0, strip.numPixels()), random(0, 255), random(0, 255), 0);
    break;
   
    case 2:
     // Green-Blue Mix
     strip.setPixelColor(random(0, strip.numPixels()), 0, random(0, 255), random(0, 255));
     
    break;
   
    case 3:
     // Blue-Red Mix
     strip.setPixelColor(random(0, strip.numPixels()), random(0, 255), 0, random(0, 255));
     
    break; 
  }
  
  // Push Update to Strip
  strip.show();
  pixelSets++;
  
}

void PulseBlue()
{
  
  unsigned char brightness = 250;
  
  char mid = strip.numPixels()/2;
  
  int currentup; 
  char currentdown;
  char decrement = 10;
  
  currentMillis = millis();
  
  if ((currentMillis - previoustime) >= 2000 | Pulseflag)
  {
    Pulseflag = 0;
    previoustime = currentMillis;

    // Set center pixel to white
    strip.setPixelColor((strip.numPixels()/2), 255, 255, 255);
    strip.show();
    
    // This for loop goes to 264 since so that the tail of the white pulse ends entirely off of the strip
    for(currentdown = strip.numPixels()/2 ,currentup = strip.numPixels()/2; currentup < 264; currentdown--, currentup++)
    {
      
        // Start at center, full brightness
        strip.setPixelColor(currentup, 250, 250, 250);
        strip.setPixelColor(currentdown, 250, 250, 250); 
        strip.show();
      
        // This goes from mid to +25 past end of strip
        currentdown--;
        currentup++;
        
        // LED brightness starts at 250, then decreases as the pixel tail length increases
        brightness = 250;
        brightness -= decrement;
  
         
        strip.setPixelColor(currentdown, brightness, brightness, brightness);
        strip.setPixelColor(currentup, brightness, brightness, brightness);
      
      
        // ok, here we go
        // We don't want the pulse tail to go over the midpoint, as this would interfere with the other pulse
        // So we check to see how long the pulse is. Lets say its currentl 119.
        // it gets to the first if statement and sees it is true. We will then decrease the brightness level for the new pixel
        // that will be added on to the end of the pulse and will then set the pixel.
        // this continues on for 24 pixels as that is the length of the pulse
        if(currentdown < mid)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 1, brightness, brightness, brightness);
          strip.setPixelColor(currentup - 1, brightness, brightness, brightness);
        } 
        
        if(currentdown < mid-1)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 2, brightness, brightness, brightness);
          strip.setPixelColor(currentup - 2, brightness, brightness, brightness);
         }
      
        if(currentdown < mid-2)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 3, brightness, brightness, brightness);
          strip.setPixelColor(currentup - 3, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-3)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 4 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 4, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-4)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 5 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 5, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-5)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 6 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 6, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-6)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 7 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 7, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-7)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 8 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 8, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-8)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 9 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 9, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-9)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 10 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 10, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-10)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 11 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 11, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-11)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 12 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 12, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-12)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 13 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 13, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-13)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 14 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 14, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-14)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 15 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 15, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-15)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 16 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 16, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-16)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 17 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 17, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-17)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 18 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 18, brightness, brightness, brightness);
        }
        
        if(currentdown < mid-18)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 19 , brightness, brightness, brightness);
          strip.setPixelColor(currentup - 19, brightness, brightness, brightness);
        }
        
        // Should return to our original blue now, which is set to 50
        if(currentdown < mid-19)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 20 , brightness, brightness, 50);
          strip.setPixelColor(currentup - 20, brightness, brightness, 50);
        }
        
        if(currentdown < mid-20)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 21 , brightness, brightness, 50);
          strip.setPixelColor(currentup - 21, brightness, brightness, 50);
        }
        
        if(currentdown < mid-21)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 22 , brightness, brightness, 50);
          strip.setPixelColor(currentup - 22, brightness, brightness, 50);
        }
        
        if(currentdown < mid-22)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 23 , brightness, brightness, 50);
          strip.setPixelColor(currentup - 23, brightness, brightness, 50);
        }
        
        if(currentdown < mid-23)
        {
          brightness -= decrement;
          strip.setPixelColor(currentdown + 24 , brightness, brightness, 50);
          strip.setPixelColor(currentup - 24, brightness, brightness, 50);
        }
        
        strip.show();
        
        
        delay(12);
      
    }
    
  }

  RandomPixelFade();
  
}

void RandomPixelFade(void)
{
  
  unsigned char pixel1 = random(0, 100);
  unsigned char pixel2 = random(100, 180);
  unsigned char pixel3 = random(180, 240);
  
  long previousMillis1 = 0;
  long previousMillis2 = 0;
  long previousMillis3 = 0;
  
  int randInterval1 = random (0, 500);
  int randInterval2 =  random (0, 500);
  int randInterval3 =  random (0, 500);
  
  // Part of the randomization process
  // Each of the 3 random pixels are checked against a random variable
  // if a is less than this random variable, then the pixel is turned on
  // This way they dont always turn on at the same time, adds some nice randomness
  
  if(random(0, 1000) < random(0, 1000))
  {
    strip.setPixelColor(pixel1, 255, 255, 255);
    strip.show();
  }
  
  if(random(0, 1000) > random(0,1000))
  {
    strip.setPixelColor(pixel2, 255, 255, 255);
    strip.show();
  }
  
  if(random(0, 1000) > random(0,1000))
  {
    strip.setPixelColor(pixel3, 255, 255, 255);
    strip.show();
  }
  
  if (millis() - previousMillis1 >= randInterval1) 
  {
    
    randInterval1 = random (0, 500);
    
    // Pixel is going to be set white, then slowly fade back to blue
    for(int x = 255; x > 0; x-=5)
    {
      
      if( x <= 50)
      {
        strip.setPixelColor(pixel1, x, x, 50);
        strip.show();
      }
      else
      {
        strip.setPixelColor(pixel1, x, x, x);
        strip.show();
      }
      delay(1);
    }
    pixel1 = random(0,100);

  }
  
  if (millis() - previousMillis2  >= randInterval2) 
  {
    randInterval2 = random (0, 500);
   
    // Pixel is going to be set white, then slowly fade back to blue
    for(int x = 255; x > 0; x-=5)
    {
      
      if( x <= 50)
      {
        strip.setPixelColor(pixel2, x, x, 50);
        strip.show();
      }
      else
      {
        strip.setPixelColor(pixel2, x, x, x);
        strip.show();
      }
      delay(1);
    }
    pixel2 = random(100,180);

  }
  
  if (millis() - previousMillis3  >= randInterval3) 
  {
    randInterval3 = random (0, 500);  

    for(int x = 255; x > 0; x-=5)
    {
      
      if( x <= 50)
      {
        strip.setPixelColor(pixel3, x, x, 50);
        strip.show();
      }
      else
      {
        strip.setPixelColor(pixel3 , x, x, x);
        strip.show();
      }
      delay(1);
    }
    pixel3 = random(180,240);

  }

}

void rainbow(uint8_t wait) {
  uint16_t i, j = 0;
 
  for(; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
	
	// We want to break if there is serial data available
	//If so, break and check command
	if(Serial.available())
		break;
		
    delay(wait);
  }
  
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
	
	// We want to break if there is serial data available
	//If so, break and check command
	if(Serial.available())
		break;
		
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
  
