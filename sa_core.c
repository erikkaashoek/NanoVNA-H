
//------------PE4302 -----------------------------------------------

// Comment out this define to use parallel mode PE4302
#define PE4302_serial

#ifdef PE4302_serial
// Clock and data pints are shared with SI4432
// Serial mode LE pin
#define PE4302_en 10
#else
//Parallel mode bit 0 pin number, according below line the PE4302 is connected to lines A0,A1,A2,A3,A4,A5
#define PE4302_pinbase A0
#endif

void PE4302_init() {
#ifdef PE4302_serial
  pinMode(PE4302_en, OUTPUT);
  digitalWrite(PE4302_en, LOW);
#else
  for (int i=0; i<6; i++) pinMode(i+PE4302_pinbase, OUTPUT);          // Setup attenuator at D6 - D11
#endif
}

void PE4302_Write_Byte(byte DATA )
{
#ifdef PE4302_serial
  //Serial mode output
  digitalWrite(SI_SCLK, LOW);
  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , DATA );
  digitalWrite(PE4302_en, HIGH);
  digitalWrite(PE4302_en, LOW);
#else
  // Parallel mode output
  for (int i=0; i<6;i++) {
    digitalWrite(i+PE4302_pinbase, p & (1<<i));
  }
#endif
}

// ---------------------------------------------------

#define MAX_VFO 3
long lFreq[MAX_VFO] = { 0,100000000,433700000};
int dataIndex = 0;


//--------------------- Frequency control -----------------------

int dirty = true;

#define STOP_MAX 430000000
#define START_MIN 0

int32_t frequency0 = 0;
int32_t frequency1 = 100000000;

static void update_frequencies(void)
{
  //  chMtxLock(&mutex_sweep);
  uint32_t start, stop;
  if (frequency1 > 0) {
    start = frequency0;
    stop = frequency1;
  } else {
    int32_t center = frequency0;
    int32_t span = -frequency1;
    start = center - span/2;
    stop = center + span/2;
  }
  lFreq[0] = start;
  lFreq[1] = stop;
  //  set_frequencies(start, stop, sweep_points);
  //  operation_requested = OP_FREQCHANGE;

  //  update_marker_index();

  // set grid layout
  //  update_grid();
  //  chMtxUnlock(&mutex_sweep);
}

static void freq_mode_startstop(void)
{
  if (frequency1 <= 0) {
    int center = frequency0;
    int span = -frequency1;
    //   ensure_edit_config();
    frequency0 = center - span/2;
    frequency1 = center + span/2;
  }
}

static void freq_mode_centerspan(void)
{
  if (frequency1 > 0) {
    int start = frequency0;
    int stop = frequency1;
    //    ensure_edit_config();
    frequency0 = (start + stop)/2; // center
    frequency1 = -(stop - start); // span
  }
}


void set_sweep_frequency(int type, int32_t freq)
{
  //  chMtxLock(&mutex_sweep);
  int32_t center;
  int32_t span;
  //  int cal_applied = cal_status & CALSTAT_APPLY;
  dirty = true;
  switch (type) {
  case ST_START:
    //    ensure_edit_config();
    freq_mode_startstop();
    if (freq < START_MIN)
      freq = START_MIN;
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    frequency0 = freq;
    // if start > stop then make start = stop
    if (frequency1 < freq)
      frequency1 = freq;
    update_frequencies();
    break;
  case ST_STOP:
    //    ensure_edit_config();
    freq_mode_startstop();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency1 = freq;
    // if start > stop then make start = stop
    if (frequency0 > freq)
      frequency0 = freq;
    update_frequencies();
    break;
  case ST_CENTER:
    //    ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency0 = freq;
    center = frequency0;
    span = -frequency1;
    if (center-span/2 < START_MIN) {
      span = (center - START_MIN) * 2;
      frequency1 = -span;
    }
    if (center+span/2 > STOP_MAX) {
      span = (STOP_MAX - center) * 2;
      frequency1 = -span;
    }
    update_frequencies();
    break;
  case ST_SPAN:
    //   ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX-START_MIN)
      freq = STOP_MAX-START_MIN;
    if (freq < 0)
      freq = 0;
    frequency1 = -freq;
    center = frequency0;
    span = -frequency1;
    if (center-span/2 < START_MIN) {
      center = START_MIN + span/2;
      frequency0 = center;
    }
    if (center+span/2 > STOP_MAX) {
      center = STOP_MAX - span/2;
      frequency0 = center;
    }
    update_frequencies();
    break;
  case ST_CW:
    //    ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency0 = freq;
    frequency1 = 0;
    update_frequencies();
    break;
  }

  //  if (cal_auto_interpolate && cal_applied)
  //    cal_interpolate(lastsaveid);
  //  chMtxUnlock(&mutex_sweep);
}

uint32_t get_sweep_frequency(int type)
{
  if (frequency1 >= 0) {
    switch (type) {
    case ST_START: return frequency0;
    case ST_STOP: return frequency1;
    case ST_CENTER: return (frequency0 + frequency1)/2;
    case ST_SPAN: return frequency1 - frequency0;
    case ST_CW: return (frequency0 + frequency1)/2;
    }
  } else {
    switch (type) {
    case ST_START: return frequency0 + frequency1/2;
    case ST_STOP: return frequency0 - frequency1/2;
    case ST_CENTER: return frequency0;
    case ST_SPAN: return -frequency1;
    case ST_CW: return frequency0;
    }
  }
  return 0;
}


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
  for (int i=0; i<DISPLAY_POINTS;i++)
    myStorage[i] = myData[i];
  settingShowStorage = true;
}

void SetClearStorage(void)
{
  settingShowStorage = false;
  settingSubtractStorage = false;
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
  dirty = true;
}


//------------------------------------------


int debug = 0;


#define DebugLine(X) { if (debug) Serial.println(X); }
#define Debug(X) { if (debug) Serial.print(X); }


int inData = 0;
long steps = DISPLAY_POINTS;
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



int peakLevel;
double peakFreq;
int peakIndex;

#define BARSTART  24


static int ownrbw = 0;
static int vbw = 0;

char *averageText[] = { "OFF", "MIN", "MAX", "2", "4", "8"};

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
    double f = ((myData[i] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
    double f2 = ((myData[i+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
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
  double f = ((((float)myData[peakIndex]) / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
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

void setup() 
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


void perform()
{
  if (autoSweepStep == 0) {
    ownrbw = settingBandwidth;
    if (ownrbw == 0)
      ownrbw = 1.2*((float)(lFreq[1] - lFreq[0]))/DISPLAY_POINTS/1000.0;

    if (ownrbw < 2.6)
      ownrbw = 2.6;
    autoSweepFreq = lFreq[0];
    autoSweepFreqStep = (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
    vbw = autoSweepFreqStep/1000.0;
    setFreq (0, lFreq[2]);
    lastFreq[0] = lFreq[2];
    startFreq = lFreq[0] + lFreq[2];
    stopFreq = lFreq[1] + lFreq[2];
    int p = - settingAttenuate * 2;
    PE4302_Write_Byte(p);
    SetPowerReference(settingPowerCal);
    SI4432_Sel = 0;
    ownrbw = SI4432_SET_RBW(ownrbw);
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
    SetRX(settingGenerate ? 3 : 0);
    peakLevel = -150;
    peakFreq = -1.0;
    SI4432_Sel=1;
    setFreq (1, lFreq[2] + autoSweepFreq + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
  }
  if (autoSweepFreqStep >0 && autoSweepStep > 0) {
    SI4432_Sel=1;
    setFreq (1, lFreq[2] + autoSweepFreq + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
  }
  SI4432_Sel=0;
  int RSSI = SI4432_RSSI();
  if (vbw > ownrbw) {
    int subSteps = ((int)(1.5 * vbw / ownrbw)) - 1;

    while (subSteps > 0) {
      //Serial.print("substeps = ");
      //Serial.println(subSteps);
      SI4432_Sel=1;
      setFreq (1, lFreq[2] + autoSweepFreq + subSteps * ownrbw * 1000 + (long)(ownrbw < 300.0?settingSpur * ownrbw * 1000:0));
      //Serial.print("Freq = ");
      //Serial.println(lFreq[2] + autoSweepFreq + subSteps * ownrbw * 1000 + (long)(ownrbw < 300.0?settingSpur * ownrbw * 1000:0));
      SI4432_Sel=0;
      int subRSSI = SI4432_RSSI();
      if (RSSI < subRSSI)
        RSSI = subRSSI;
      subSteps--;
    }
  }
  if (settingShowStorage)
    if (settingSubtractStorage)
      RSSI = 128 + RSSI - myStorage[autoSweepStep] ;
  if (dirty || settingAverage == AV_OFF)
    myData[autoSweepStep] = (unsigned char) RSSI;
  else {
    switch(settingAverage) {
    case AV_MIN: if (myData[autoSweepStep] > (unsigned char) RSSI) myData[autoSweepStep] = (unsigned char) RSSI; break;
    case AV_MAX: if (myData[autoSweepStep] < (unsigned char) RSSI) myData[autoSweepStep] = (unsigned char) RSSI; break;
    case AV_2: myData[autoSweepStep] = (myData[autoSweepStep] + RSSI) / 2; break;
    case AV_4: myData[autoSweepStep] = (myData[autoSweepStep]*3 + RSSI) / 4; break;
    case AV_8: myData[autoSweepStep] = (myData[autoSweepStep]*7 + RSSI) / 8; break;
    }
    myActual[autoSweepStep] = RSSI;
  }
  if (autoSweepFreq > 1000000) {
    if (peakLevel < myData[autoSweepStep]) {
      peakIndex = autoSweepStep;
      peakLevel = myData[autoSweepStep];
      peakFreq = autoSweepFreq;
    }
  }
  if (myData[autoSweepStep] == 0) {
    SI4432_Init();
  }
  if (!settingGenerate || autoSweepStep == 0) {
    autoSweepStep++;
    autoSweepFreq += (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
  }
  if (autoSweepStep >= DISPLAY_POINTS) {
    if (settingAverage && dirty)
      dirty = false;
    autoSweepStep = 0;
    settingSpur = -settingSpur;
  }
}


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
