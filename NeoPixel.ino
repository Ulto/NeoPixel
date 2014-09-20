/*******************************
 * CrossFire Neopixels  + Greg *
 ******************************/
 
 // V1.3
 
 /* CUSTOMIZABLE PARAMETERS */
   // Neopixel LED Strip
   #define NumPixels 240                    // number of pixels in strip (255 MAX)
   #define DataOut 8                        // pin number (most are valid)
   #define BG 1  //unsure right now
   
   #define PixelInfo NEO_GRB + NEO_KHZ800   // pixel type flags, add together as needed:
                                                //   NEO_RGB     Pixels are wired for RGB bitstream
                                                //   NEO_GRB     Pixels are wired for GRB bitstream
                                                //   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
                                                //   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
                                                
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel. Avoid connecting
// on a live circuit...if you must, connect GND first.
                                                
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

int color;
int center = 0;
int step = -1;
int maxSteps = 16;
float fadeRate = 1;
int diff;

char ver[] = {1.30};

//background color
uint32_t currentBg = random(256);
uint32_t nextBg = currentBg;

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
      Pulse(0, 0, 50, 250, 250, 250); //Blue - Pulse White
    break;
    
    case 7:
      Pulse(0, 0, 50, 250, 0, 0); //Blue - Pulse Red
    break;
    
    case 8:
      Pulse(0, 0, 50, 0, 250, 0); //Blue - Pulse Green
    break;
   
    case 9:
      Pulse(140, 0, 0, 250, 250, 250); //Red - Pulse White
    break;
    
    case 10:
      Pulse(140, 0, 0, 0, 0, 250); //Red - Pulse Blue
    break;
    
    case 11:
      Pulse(140, 0, 0, 0, 250, 0); //Red - Pulse Green
    break;  
  
    case 12:
      Pulse(0, 50, 0, 250, 250, 250); //Green - Pulse White
    break;
    
    case 13:
      Pulse(0, 50, 0, 0, 0, 250); //Green - Pulse Blue
    break;
    
    case 14:
      Pulse(0, 50, 0, 250, 0, 0); //Green - Pulse Red
    break;    
    
    case 15:
      rainbow(20);
    break;
    
    case 16:
      rainbowCycle(20);
    break;
    
    case 17:
      RandomPixelFade();
    break;
    
    case 18:
      theaterChase(strip.Color(0,   0, 127), 50); // Blue
    break;
    
    case 19:
      theaterChase(strip.Color(127,   0,   0), 50); // Red
    break;
    
    case 20:
      theaterChase(strip.Color(0,   127,  0), 50); // Green
    break;
    
    case 21:
      theaterChase(strip.Color(127,   127, 127), 50); // White
    break;
    
    case 22:
      theaterChaseRainbow(50);
    break;
     
    case 23:
      ripple();
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


void printMenu() {
  Serial.println(F("****************************"));
  Serial.println(F(" CrossFire Neopixels + Greg "));
  Serial.println(F("****************************"));
  Serial.println();
  Serial.println(F(" Please Select a Show Type: "));
  Serial.println(F(" 0 - Lights Off & Re-Print Menu"));
  Serial.println(F(" 1 - Traditional, Randon pixels set to random vibrant colors."));
  Serial.println(F(" 2 - Traditional + Kill, For every pixel that is set, another is blacked out."));
  Serial.println(F(" 3 - Traditional RGB Only"));
  Serial.println(F(" 4 - Traditional RGB Only + Kill, For every pixel that is set, another is blacked out."));
  Serial.println(F(" 5 - Spectrum"));
  Serial.println(F(" 6 - Blue - White Pulse"));
  Serial.println(F(" 7 - Blue - Red Pulse"));
  Serial.println(F(" 8 - Blue - Green Pulse"));
  Serial.println(F(" 9 - Red - White Pulse"));
  Serial.println(F(" 10 - Red - Blue Pulse"));
  Serial.println(F(" 11 - Red - Green Pulse"));  
  Serial.println(F(" 12 - Green - White Pulse"));
  Serial.println(F(" 13 - Green - Blue Pulse"));
  Serial.println(F(" 14 - Green - Red Pulse"));  
  Serial.println(F(" 15 - Rainbow"));
  Serial.println(F("16 - Rainbow Cycle")); 
  Serial.println(F("17 - Random Pixel Fade"));
  Serial.println(F("18 - Blue Theater Chase"));
  Serial.println(F("19 - Green Theater Chase"));
  Serial.println(F("20 - Red Theater Chase"));
  Serial.println(F("21 - White Theater Chase"));
  Serial.println(F("22 - Rainbow Theater Chase"));
  Serial.println(F("23 - Ripple"));
  Serial.println(F("99 - View Pixel Set Counter (Current Show Setting Retained)"));
  Serial.println();
  Serial.println(F("Show Selection: ")); 
  Serial.flush();
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
     
     case 6:  // 8 - Blue- White Pulse
       menuSelection = 6;
       Serial.println("Option 6 Selected");
     break;
     
     case 7:  //7 - Blue - Red Pulse
       menuSelection = 7;
       Serial.println("Option 7 Selected");
     break;
     
     case 8:  // 8 - Blue- Green Pulse
       menuSelection = 8;
       Serial.println("Option 8 Selected");
     break;
     
     case 9:  // 9 - Red - White Pulse
       menuSelection = 9;
       Serial.println("Option 9 Selected");
     break;
     
     case 10:  //10 - Red - Blue Pulse
       menuSelection = 10;
       Serial.println("Option 10 Selected");
     break;
     
     case 11:  // 11 - Red- Green Pulse
       menuSelection = 11;
       Serial.println("Option 11 Selected");
     break;
     
     case 12:  // 12 - Green - White Pulse
       menuSelection = 12;
       Serial.println("Option 12 Selected");
     break;
     
     case 13:  //13 - Green - Blue Pulse
       menuSelection = 13;
       Serial.println("Option 13 Selected");
     break;
     
     case 14:  // 14 - Green - Red Pulse
       menuSelection = 14;
       Serial.println("Option 14 Selected");
     break;
     
     case 15:  // 15 - Rainbow
       menuSelection = 15;
       Serial.println("Option 15 Selected");
     break;
     
     case 16:  // 16 - Rainbow Cycle
       menuSelection = 16;
       Serial.println("Option 16 Selected");
     break;
     
     case 17:  // 17 - Random Pixel Fade
       menuSelection = 17;
       Serial.println("Option 17 Selected");
     break;
     
     case 18:  // 18 - Blue Theater Chase
       menuSelection = 18;
       Serial.println("Option 18 Selected");
     break;
     
     case 19:  // 19 - Green Theater Chase
       menuSelection = 19;
       Serial.println("Option 19 Selected");
     break;
     
     case 20:  // 20 - Red Theater Chase
       menuSelection = 20;
       Serial.println("Option 20 Selected");
     break;
     
     case 21:  // 21 - White Theater Chase
       menuSelection = 21;
       Serial.println("Option 21 Selected");
     break;
     
     case 22:  // 22 - Rainbow Theater Chase
       menuSelection = 22;
       Serial.println("Option 22 Selected");
     break;
     
     case 23:  // 23 - Rainbow Theater Chase
       menuSelection = 23;
       Serial.println("Option 23 Selected");
     break;
     
     
     case 99:  // 9 - View Pixel Set Counter (Current Show Setting Retained)
       Serial.print("Pixel Sets Since Start: ");
       Serial.println(pixelSets);
     break;
     
     default:
       Serial.println("INVALID INPUT");
     break;
     
   }
   
   Serial.flush();
   
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

void Pulse(uint8_t R_Bright, uint8_t G_Bright, uint8_t B_Bright, uint8_t R_Pulse, uint8_t G_Pulse, uint8_t B_Pulse){

  char mid = strip.numPixels()/2;
  uint8_t saveR, saveG, saveB;
  int x;
  int currentup; 
  char currentdown;
  char decrement = 10;
  
  saveR = R_Pulse;
  saveG = G_Pulse;
  saveB = B_Pulse;
  
  currentMillis = millis();
  
  if ((currentMillis - previoustime) >= 2000 | Pulseflag)
  {
    Pulseflag = 0;
    previoustime = currentMillis;

    strip.setPixelColor((strip.numPixels()/2), R_Pulse, G_Pulse, B_Pulse);
    strip.show();
    
    // This for loop goes to 264 since so that the tail of the white pulse ends entirely off of the strip
    for(currentdown = strip.numPixels()/2 ,currentup = strip.numPixels()/2; currentup < (strip.numPixels()+24); currentdown--, currentup++)
    {
      // Restore the pulse values for the loop
        R_Pulse = saveR;
        G_Pulse = saveG;
        B_Pulse = saveB;
        
        // Start at center, full brightness
        strip.setPixelColor(currentup, R_Pulse, G_Pulse, B_Pulse);
        strip.setPixelColor(currentdown, R_Pulse, G_Pulse, B_Pulse);
        strip.show();
      
        // This goes from mid to +25 past end of strip
        currentdown--;
        currentup++;
        
       if(R_Pulse)
         R_Pulse -= decrement;
         
       if(G_Pulse)
         G_Pulse -= decrement;
        
       if(B_Pulse)
         B_Pulse -= decrement;
  
        strip.setPixelColor(currentdown,  R_Pulse, G_Pulse, B_Pulse);
        strip.setPixelColor(currentup,  R_Pulse, G_Pulse, B_Pulse);
      
      
        for(int i = 0; i < 23; i++){
        
          if(currentdown < mid - i)
          {
            if(i >= 19){
              if(R_Bright > 0){
                if(G_Pulse)
                  G_Pulse -= decrement;
                if(B_Pulse)
                  B_Pulse -= decrement;
                strip.setPixelColor(currentdown + i+1, R_Bright, G_Pulse, B_Pulse); 
                strip.setPixelColor(currentup - i-1, R_Bright, G_Pulse, B_Pulse); 
                
              }
              else if(G_Bright > 0){
                if(R_Pulse)
                  R_Pulse -= decrement;
                if(B_Pulse)
                  B_Pulse -= decrement;
                strip.setPixelColor(currentdown + i+1, R_Pulse, G_Bright, B_Pulse); 
                strip.setPixelColor(currentup - i-1, R_Pulse, G_Bright, B_Pulse); 
                
              }
              else if(B_Bright > 0){
                if(R_Pulse)
                  R_Pulse -= decrement;
                if(G_Pulse)
                  G_Pulse -= decrement;
                strip.setPixelColor(currentdown + i+1, R_Pulse, G_Pulse, B_Bright); 
                strip.setPixelColor(currentup - i-1, R_Pulse, G_Pulse, B_Bright); 
                
              }
          
              
            }
            else{
             if(R_Pulse)
               R_Pulse -= decrement;
               
             if(G_Pulse)
               G_Pulse -= decrement;
              
             if(B_Pulse)
               B_Pulse -= decrement;
              strip.setPixelColor(currentdown + i+1, R_Pulse, G_Pulse, B_Pulse); 
              strip.setPixelColor(currentup - i-1, R_Pulse, G_Pulse, B_Pulse);
            }
          }
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
  static uint16_t i, j;
 
 if(j < 256){
   
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
	
    j++;
    delay(wait);
  }
  else
    j = 0;
  
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  static uint16_t i, j;

  if(j < (256*5)){
    
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
		
    delay(wait);
    j++;
  }
  else
    j = 0;
  
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  static int j;
  if(j < 256){     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
    j++;
  }
  else
    j = 0;
}

void ripple() {
  if (BG){
    if (currentBg == nextBg) {
      nextBg = random(256);
    }
    else if (nextBg > currentBg) {
      currentBg++;
    } else {
      currentBg--;
    }
    for(uint16_t l = 0; l < NumPixels; l++) {
      strip.setPixelColor(l, Wheel2(currentBg, 0.1));
    }
  } 
  else {
      for(uint16_t l = 0; l < NumPixels; l++) {
        strip.setPixelColor(l, 0, 0, 0);
      }
  }
    
  if (step == -1) {
      center = random(NumPixels);
      color = random(256);
      step = 0;
  }
    
    if (step == 0) {
      strip.setPixelColor(center, Wheel2(color, 1));
      step ++;
    }
    else {
      if (step < maxSteps) {
        strip.setPixelColor(wrap(center + step), Wheel2(color, pow(fadeRate, step)));
        strip.setPixelColor(wrap(center - step), Wheel2(color, pow(fadeRate, step)));
        if (step > 3) {
          strip.setPixelColor(wrap(center + step - 3), Wheel2(color, pow(fadeRate, step - 2)));
          strip.setPixelColor(wrap(center - step + 3), Wheel2(color, pow(fadeRate, step - 2)));
        }
        step ++;
      }
      else {
        step = -1;
      }
    }
    strip.show();
    delay(45);
}
 
 
int wrap(int step) {
  
  if(step < 0)
    return NumPixels + step;
    
  if(step > NumPixels - 1) 
    return step - NumPixels;
  
  return step;
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

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel2(byte WheelPos, float opacity)
{
    if(WheelPos < 85) {
        return strip.Color((WheelPos * 3) * opacity, (255 - WheelPos * 3) * opacity, 0);
    }
    else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color((255 - WheelPos * 3) * opacity, 0, (WheelPos * 3) * opacity);
    }
    else {
        WheelPos -= 170;
        return strip.Color(0, (WheelPos * 3) * opacity, (255 - WheelPos * 3) * opacity);
    }
}
  
