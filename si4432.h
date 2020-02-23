#ifndef __SI4432_H__
#define __SI4432_H__

#define byte uint8_t
extern int SI4432_Sel;         // currently selected SI4432
void SI4432_Write_Byte(byte ADR, byte DATA );
byte SI4432_Read_Byte( byte ADR );

void SI4432_Init(void);
float SI4432_RSSI(void);
void SI4432_Set_Frequency ( long Freq );
float SI4432_SET_RBW(float WISH);

#endif

#endif //__SI4432_H__
