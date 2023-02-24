/*
*********************************************************************************************************
*
*	Nombre del módulo: módulo de relación de letras verdes
*	Nombre del archivo : Split.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Split.h"


SplitType       SplitNow;   //Datos de relación de letras verdes actualmente en ejecución
SplitTable      SplitTab;   //Tabla de relación de letras verdes
PhaseSplitType  RingSplit[RingMax];    //Relación de tiempo a señal verde de cada anillo




//Número de fase + datos GSR = índice de fase GSR
uint8_t GetSplitPhaseIndex(SplitType* Split, uint8_t PhaseNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < PhaseMax; i++)
    {
        if(Split->Phase[i].PhaseNum == PhaseNum)
        {
            temp = i;
            break;
        }
    }
    
    return temp;
}

void SplitDefault(void)
{
    uint8_t i;
    
    memset(&SplitTab.Maximum,0x00,sizeof(SplitTab));
    SplitTab.Maximum = SplitMax;

    SplitTab.Split[0].Num = 1;
    for(i = 0; i <= 4; i++)//5 fases están configuradas por defecto
    {
        SplitTab.Split[0].Phase[i].PhaseNum = i + 1;//
        SplitTab.Split[0].Phase[i].Time = 16;       //Tiempo de liberación de fase. Contiene luz verde, flash verde, luz amarilla, tiempo rojo completo
        SplitTab.Split[0].Phase[i].Mode = SM_None;  //Relación de señal verde Modo 2-Ninguno
        SplitTab.Split[0].Phase[i].Coord = SC_FIXED;//Configuración del coordinador Coord bit2: 1-como fase fija
    }
}

//Configuración de proporción de letra verde predeterminada
//La configuración predeterminada de Green Letter Ratio 1
void SplitDataInit(uint8_t n)
{
    uint8_t i;
    SplitTab.Split[n].Num = n + 1;
    for(i = 0; i <= 4; i++)//5 fases están configuradas por defecto
    {
        SplitTab.Split[n].Phase[i].PhaseNum = i + 1;//
        SplitTab.Split[n].Phase[i].Time = 18;       //Tiempo de liberación de fase. Contiene luz verde, flash verde, luz amarilla, tiempo rojo completo
        SplitTab.Split[n].Phase[i].Mode = SM_None;  //Relación de señal verde Modo 2-Ninguno
        SplitTab.Split[n].Phase[i].Coord = SC_FIXED;//Configuración del coordinador Coord bit2: 1-como fase fija
    }
}

void SplitXDataInit(SplitType* Split)
{
    uint8_t n;
    Split->Num = 1;
    for(n=0;n<=4;n++)
    {
        Split->Phase[n].PhaseNum = n + 1;//
        Split->Phase[n].Time = 18;      //Tiempo de liberación de fase. Incluyendo la luz verde, el destello verde, la luz amarilla y el tiempo rojo 
                                            //completo de la fase del vehículo de motor Y el tiempo de liberación y tiempo de despeje de la fase peatonal.
        Split->Phase[n].Mode = SM_None;  //Letra verde que modo 2 - no
        Split->Phase[n].Coord = SC_FIXED;//Configuración del coordinador Coord bit2: 1-como fase fija
    }
}

