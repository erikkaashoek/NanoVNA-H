
int SI4432_Sel = 0;

//------------PE4302 -----------------------------------------------

// Comment out this define to use parallel mode PE4302

#define PE4302_en 10

void PE4302_init() {
//  pinMode(PE4302_en, OUTPUT);
//  digitalWrite(PE4302_en, LOW);
}

void PE4302_Write_Byte(unsigned char DATA )
{
  //Serial mode output
//  digitalWrite(SI_SCLK, LOW);
//  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , DATA );
//  digitalWrite(PE4302_en, HIGH);
//  digitalWrite(PE4302_en, LOW);
}

// ---------------------------------------------------

//-----------------SI4432 dummy------------------
void SI4432_Write_Byte(unsigned char ADR, unsigned char DATA ) {}
float SI4432_SET_RBW(float WISH) {return WISH;}
void SI4432_SetPowerReference(int p) {}
void SI4432_Set_Frequency(long f) {}

unsigned long seed = 123456789;

double myfrand()
{
  seed = (unsigned int) (1103515245 * seed + 12345) ;
  return ((double) seed) / 1000000000.0;
}
#define NOISE  (myfrand() * 0.001)
double SI4432_RSSI(int i)
{
  if (frequencies[i] > 10000000 && frequencies[i] < 11000000)
    return(1.0 + NOISE);
  return 0.00001 + NOISE;
}
void SI4432_Init(void) {}
//--------------------- Frequency control -----------------------

int dirty = true;

//---------------- menu system -----------------------

int settingMax = -10; //9 drids vertical
int settingMin = -100;
int settingAttenuate = 0;
int settingGenerate = 0;
int settingBandwidth = 0;
int settingLevelOffset = 0;
int settingPowerCal = 1;
int settingPowerGrid = 10;
int settingSpur = 0;
int settingAverage = 0;
int settingShowStorage = 0;
int settingSubtractStorage = 0;


void set_refer_output(int v)
{
  settingPowerCal = v;
  dirty = true;
}

void SetRefLevel(int ref)
{
  settingMin = ref - (settingMax - settingMin);
  settingMax =ref;
  dirty = true;
}

void SetGenerate(int g)
{
  settingGenerate = g;
  dirty = true;
}

void SetPowerGrid(int g)
{
  settingPowerGrid = g;
  settingMin = settingMax - 9*g;
  dirty = true;
}

void SetAttenuation(int a)
{
  settingAttenuate = -a;
  dirty = true;
}

void SetStorage(void)
{
  for (int i=0; i<POINT_COUNT;i++)
    stored_t[i] = actual_t[i];
  settingShowStorage = true;
  trace[TRACE_STORED].enabled = true;
}

void SetClearStorage(void)
{
  settingShowStorage = false;
  settingSubtractStorage = false;
  trace[TRACE_STORED].enabled = false;
}

void SetSubtractStorage(void)
{
  if (!settingShowStorage)
    SetStorage();
  settingSubtractStorage = true;
}

extern int peakLevel;
void SetPowerLevel(int o)
{
  if (o != 100)
    settingLevelOffset = o - (int)((peakLevel/ 2.0  - settingAttenuate) - 120.0);
  else
    settingLevelOffset = 0;
  dirty = true;
}

void SetRBW(int v)
{
  settingBandwidth = v;
  dirty = true;
}

void SetSpur(int v)
{
  settingSpur = v;
  dirty = true;
}

void SetAverage(int v)
{
  settingAverage = v;
  trace[TRACE_TEMP].enabled = (v != 0);
  dirty = true;
}


//------------------------------------------


int peakLevel;
double peakFreq;
int peakIndex;

#define BARSTART  24


static float ownrbw = 0;
static float vbw = 0;


int inData = 0;
unsigned long  startFreq = 250000000;
unsigned long  stopFreq = 300000000;
unsigned long  lastFreq[6] = { 300000000, 300000000,0,0,0,0};
int lastParameter[10];
int parameter;
int VFO = 0;
int RX = 2;
int extraVFO=-1;
int extraVFO2 = -1;
unsigned long reg = 0;
long offset=0;
long offset2=0;
static unsigned int spacing = 10000;
double delta=0.0;
int phase=0;
int deltaPhase;
int delaytime = 50;
int drive = 6;
unsigned int sensor;
int hardware = 0;



#if 0
void displayHisto ()
{
  //  clearDisplay();
  //int settingMax = 0;
  //int settingMin = -120;

  if (old_settingMax != settingMax || old_settingMin != settingMin) {
    // Display levels at left of screen
    tft.fillRect(0, 0, oX-2, tft.height(), DISPLAY_BLACK);
    textWhite();
    tft.setCursor(0,oY);             // Start at top-left corner
    tft.println(settingMax);
    tft.setCursor(0,tft.height() - 16);
    tft.println(settingMin);
    //  tft.setCursor(0,tft.height()/2);
    //  tft.println("dB");
    old_settingMax = settingMax;
    old_settingMin = settingMin;
  }

  if (old_startFreq != startFreq || old_stopFreq != stopFreq) {
    // Dsiplay frequencies
    // Bottom of screen
    tft.fillRect(0, tft.height()-8, tft.width(), tft.height()-1, DISPLAY_BLACK);
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.setCursor(oX+2,tft.height()-8);             // Start at top-left corner
    double f = (((double)(startFreq - lastFreq[0]))/ 1000000.0);
    tft.print(f);
    tft.print("MHz");
    tft.setCursor(tft.width() - 58,tft.height()-8);
    f = (((double)(stopFreq - lastFreq[0]))/ 1000000.0);
    tft.print(f);
    tft.print("MHz");

    tft.setCursor(tft.width()/2 - 80 + oX,tft.height()-8);
    tft.print("center:");
    f = (double)((stopFreq/2 + startFreq/2 - lastFreq[0]) / 1000000.0);
    tft.print(f);
    tft.print("MHz");
    old_startFreq = startFreq;
    old_stopFreq = stopFreq;
  }

  // Top of screen

  if (old_settingAttenuate != settingAttenuate || old_settingPowerGrid != settingPowerGrid) {
    tft.fillRect(0, 0, 8*6, oY-2, DISPLAY_BLACK);
    tft.setCursor(0,0);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print("Atten:");
    tft.print(settingAttenuate);
    tft.setCursor(0,8);             // Start at top-left corner
    tft.print(settingPowerGrid);
    tft.print("dB/");
    old_settingAttenuate = settingAttenuate;
    old_settingPowerGrid = settingPowerGrid;
    old_ownrbw = -1;
  }  

  if (old_ownrbw != ownrbw || old_vbw != vbw) {
    tft.fillRect(56, 0, 99, oY-2, DISPLAY_BLACK);
    tft.setCursor(56,0);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print("RBW:");
    tft.print(ownrbw);
    tft.print("kHz");
    tft.setCursor(56,8);             // Start at top-left corner
    tft.print("VBW:");
    tft.print(vbw);
    tft.print("kHz");
    old_ownrbw = ownrbw;
    old_vbw = vbw;
  }  

  if (peakLevel > -150) {
    tft.fillRect(oX+100, 0, 100, 8-1, DISPLAY_BLACK);
    tft.setCursor(oX + 100,0);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print("Max=");
    tft.print((int)((peakLevel/ 2.0  - settingAttenuate) - 120.0)+settingLevelOffset);
    tft.print("dB, ");
    tft.print(peakFreq/ 1000000.0);
    tft.print("MHz");
  }

  if (old_settingAverage != settingAverage || abs(old_settingSpur) != abs(settingSpur)) {
    int x =  tft.width() - 60;
    tft.fillRect( x, 0, 60, oY-2, DISPLAY_BLACK);
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    if (settingAverage) {
      tft.setCursor( x,0);             // Start at top-left corner
      tft.print("AVR:");
      tft.print(averageText[settingAverage]);
    }
    if (settingSpur) {
      tft.setCursor(x,8);             // Start at top-left corner
      tft.print("SPUR:");
      tft.print("ON");
    }
    old_settingAverage = settingAverage;
    old_settingSpur = settingSpur;
  }  



  /*
  for (int i=0; i<DISPLAY_POINTS - 1; i++) {
    int delta=settingMax - settingMin;
    DrawCheckerBoard(i);
    double f = ((actual_t[i] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
    double f2 = ((actual_t[i+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f2 = (f2 - settingMin) * Y_GRID * dY / delta;
    if (f2 >= Y_GRID * dY) f2 = Y_GRID * dY-1;
    if (f2 < 0) f2 = 0;
  int x = i;
  int Y1 = Y_GRID * dY - 1 - (int)f;
  int Y2 = Y_GRID * dY - 1 - (int)f2;
  tft.drawLine(x+oX, oY+Y1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
//  tft.drawLine(x+oX, oY+Y1+1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
  }


   */
  sendDisplay();
}

void DisplayPoint(unsigned char *data, int i, int color)
{
  if (i == 0)
    return;
  int x = i-1;
  int delta=settingMax - settingMin;
  double f = ((data[x] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
  f = (f - settingMin) * Y_GRID * dY / delta;
  if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
  if (f < 0) f = 0;
  double f2 = ((data[x+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
  f2 = (f2 - settingMin) * Y_GRID * dY / delta;
  if (f2 >= Y_GRID * dY) f2 = Y_GRID * dY-1;
  if (f2 < 0) f2 = 0;
  int Y1 = Y_GRID * dY - 1 - (int)f;
  int Y2 = Y_GRID * dY - 1 - (int)f2;
  DrawDirty(x,min(Y2,Y1));
  DrawDirty(x+1,min(Y2,Y1));
  tft.drawLine(x+oX, oY+Y1, x+oX+1, oY+Y2, color);
  //  tft.drawLine(x+oX, oY+Y1+1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
  sendDisplay();
}

void DisplayPeakData(void)
{
  double f = ((((float)actual_t[peakIndex]) / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
  int delta=settingMax - settingMin;
  f = (f - settingMin) * Y_GRID * dY / delta;
  if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
  if (f < 0) f = 0;
  int Y1 = Y_GRID * dY - 1 - (int)f;
  tft.setCursor(oX+peakIndex+5,oY+Y1);             // Start at top-left corner
  tft.setTextColor(DISPLAY_WHITE);        // Draw white text
  tft.print(peakFreq/ 1000000.0);
  tft.setCursor(oX+peakIndex+5,oY+Y1+8);             // Start at top-left corner
  tft.print((int)((peakLevel/ 2.0  - settingAttenuate) - 120.0)+settingLevelOffset);
  tft.print("dB");
  for (int x=peakIndex+5;x<peakIndex+5+6*8;x++)
    DrawDirty(x,Y1);
}

#endif

void setupSA() 
{
  SI4432_Init();
  PE4302_init();
  PE4302_Write_Byte(0);
}


void setFreq(int V, unsigned long freq)
{
  if (V>=0) {
    SI4432_Sel = V;
#ifdef USE_SI4463
    if (SI4432_Sel == 2) {
      freq = freq - 433000000;
      freq = freq / 10000;  //convert to 10kHz channel starting with 433MHz
      //      Serial.print("Set frequency Si4463 = ");
      //      Serial.println(freq);
      Si446x_RX ((uint8_t)freq);
    }
    else
#endif
      SI4432_Set_Frequency(freq);
  }
}

void SetRX(int p)
{
  RX = p;
  if (RX == 3) {  //Both on RX
    SI4432_Sel = 0;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1F);//Set low power
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1F);//Set full power
  } else {
    if (RX == 0) {
      SI4432_Sel = 0;
      SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

      SI4432_Sel = 1;
      SI4432_Write_Byte(0x7, 0x0B); // start TX
      SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power

    } else if (RX == 1) {
      SI4432_Sel = 0; // both as receiver to avoid spurs
      SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

      SI4432_Sel = 1;
      SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

    } else if (RX == 2) { // SI4463 as receiver
      SI4432_Sel = 0;
      SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

      SI4432_Sel = 1;
      SI4432_Write_Byte(0x7, 0x0B); // start TX
      SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
    }
#if 0 // compact
    SI4432_Sel = (RX ? 1 : 0);
    SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

    SI4432_Sel = (RX ? 0 : 1);
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
#endif
  }

}

int autoSweepStep = 0;
long autoSweepFreq = 0;
long autoSweepFreqStep = 0;
int standalone = true;


void perform(int i)
{
  if (i == 0) {
    ownrbw = settingBandwidth;
    if (ownrbw == 0)
      ownrbw = 1.2*((float)(frequencies[1] - frequencies[0]))/1000.0;

    if (ownrbw < 2.6)
      ownrbw = 2.6;
    autoSweepFreq = frequencies[0];
    autoSweepFreqStep = (frequencies[1] - frequencies[0])/POINT_COUNT;
    vbw = autoSweepFreqStep/1000.0;
    setFreq (0, frequency_IF);
    int p = - settingAttenuate * 2;
    PE4302_Write_Byte(p);
    SI4432_SetPowerReference(settingPowerCal);
    SI4432_Sel = 0;
    ownrbw = SI4432_SET_RBW(ownrbw);
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
    SetRX(settingGenerate ? 3 : 0);
    peakLevel = -150;
    peakFreq = -1.0;
    SI4432_Sel=1;
    setFreq (1, frequency_IF + autoSweepFreq + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
  }
  if (autoSweepFreqStep >0 && i > 0) {
    SI4432_Sel=1;
    setFreq (1, frequency_IF + frequencies[i] + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
  }
  SI4432_Sel=0;
  double RSSI = SI4432_RSSI(i);
  if (vbw > ownrbw) {
    int subSteps = ((int)(1.5 * vbw / ownrbw)) - 1;

    while (subSteps > 0) {
      //Serial.print("substeps = ");
      //Serial.println(subSteps);
      SI4432_Sel=1;
      setFreq (1, frequency_IF + frequencies[i] + subSteps * ownrbw * 1000 + (long)(ownrbw < 300.0?settingSpur * ownrbw * 1000:0));
       SI4432_Sel=0;
      double subRSSI = SI4432_RSSI(i);
      if (RSSI < subRSSI)
        RSSI = subRSSI;
      subSteps--;
    }
  }
  temp_t[i] = RSSI;
  if (settingShowStorage) {
    if (settingSubtractStorage) {
      RSSI = RSSI - stored_t[i] ;
    }
  }
  if (dirty || settingAverage == AV_OFF)
    actual_t[i] = RSSI;
  else {
    switch(settingAverage) {
    case AV_MIN: if (actual_t[i] > RSSI) actual_t[i] = RSSI; break;
    case AV_MAX: if (actual_t[i] < RSSI) actual_t[i] = RSSI; break;
    case AV_2: actual_t[i] = (actual_t[i] + RSSI) / 2.0; break;
    case AV_4: actual_t[i] = (actual_t[i]*3 + RSSI) / 4.0; break;
    case AV_8: actual_t[i] = (actual_t[i]*7 + RSSI) / 8.0; break;
    }
  }
  if (autoSweepFreq > 1000000) {
    if (peakLevel < actual_t[i]) {
      peakIndex = i;
      peakLevel = actual_t[i];
      peakFreq = autoSweepFreq;
    }
  }
  if (actual_t[i] == 0) {
    SI4432_Init();
  }
  if (i = POINT_COUNT -1) {
    if (settingAverage && dirty)
      dirty = false;
    settingSpur = -settingSpur;
  }
}

#if 0
void int WriteReadRegister() {
  if(inData == 'X' || inData == 'x')
  {
    char t[40];
    int i = 0;
    int reg;
    int addr;
    char c = 0;
    delay(1);
    while (Serial.available() > 0 && c != ' ') {
      delay(1);
      c = Serial.read();  //gets one byte from serial buffer
      t[i++] = c; //makes the string readString
    }
    t[i++] = 0;
    addr = strtoul(t, NULL, 16);
    i = 0;
    while (Serial.available() > 0) {
      delay(1);
      c = Serial.read();  //gets one byte from serial buffer
      t[i++] = c; //makes the string readString
    }
    t[i++] = 0;
    SI4432_Sel = VFO;
    if (i == 1) {
      Serial.print("Reg[");
      Serial.print(addr, HEX);
      Serial.print("] : ");
      Serial.println(SI4432_Read_Byte(addr), HEX);
    } else {
      reg = strtoul(t, NULL, 16);
      Serial.print("Reg[");
      Serial.print(addr, HEX);
      Serial.print("] = ");
      Serial.println(reg, HEX);
      SI4432_Write_Byte(addr, reg);
    }
    inData = 0;
  }
}
#endif
