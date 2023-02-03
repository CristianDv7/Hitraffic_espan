#ifndef SPLIT_H
#define SPLIT_H
#include "public.h"


/*
  1. Tiempo de fase SplitTime 0-255
        Tiempo de liberaci�n de fase. Incluyendo la luz verde, el destello verde, la luz amarilla y el tiempo rojo completo de la fase del veh�culo de motor
        Y el tiempo de liberaci�n y tiempo de despeje de la fase peatonal.
    2. Modo de relaci�n de se�al verde SplitMode
    Bandera de 1 variable: la fase se emite como una bandera variable en este modo de relaci�n de se�al verde
    2-ninguno
    3- Respuesta m�nima del veh�culo:
        Cuando el control es inductivo, la fase del veh�culo se aplica en verde m�nimo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los par�metros de fase.
    4- Respuesta m�xima del veh�culo:
        Cuando se controla de forma inductiva, la fase del veh�culo motorizado se aplica al verde m�ximo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los par�metros de fase.
    5- Respuesta peatonal:
        Durante el control de inducci�n, la fase peatonal est� obligada a obtener el derecho de liberaci�n.
        Este atributo tiene prioridad sobre el atributo "solicitud autom�tica de peatones" en el par�metro de fase.
    6- Respuesta m�xima de veh�culos/peatones:
        En el control de inducci�n, la fase de veh�culos de motor est� obligada a implementar el verde m�ximo, y la fase de peatones est� obligada a obtener el derecho de liberaci�n.
        Esta propiedad tiene una prioridad m�s alta que la propiedad "solicitud autom�tica de veh�culos" y la propiedad "solicitud autom�tica de peatones" en los par�metros de fase.
    7- Ignorar fase
        Esta fase se elimina del esquema en este modo de relaci�n de se�al verde.
    3. Configuraci�n del coordinador Coord
        bit0: 1 - Cuando el control es coordinado, esta fase se usa como una fase coordinada para coordinar con otras intersecciones.
        bit1: 1- como fase clave
        bit2: 1-como fase fija
*/
typedef enum 
{
    SM_Other = 1,
    SM_None = 2,
    SM_MinVehRecall = 3,
    SM_MaxVehRecall = 4,
    SM_PedRecall = 5,
    SM_MaxVehPedRecall = 6,
    SM_Omitted = 7,
}SplitMode;

#define SC_NONE       0x00
#define SC_COORD      0x01
#define SC_KEY        0x02
#define SC_FIXED      0x04

//��λʱ��  ��λ�ķ���ʱ�䡣
//�����˻�������λ���̵ơ��������Ƶơ�ȫ��ʱ��
//�Լ�������λ�ķ���ʱ������ʱ�䡣
typedef struct
{
    uint8_t PhaseNum;           //��λ��
    uint8_t Time;               //��λʱ��
    uint8_t Mode;               //��λģʽ
    uint8_t Coord;              //Э������ 0-1 
}PhaseSplitType; //��λ���űȶ���

typedef struct
{
    uint8_t         Num;     //���űȺ�
    PhaseSplitType  Phase[PhaseMax];    //16
}SplitType;    //���ű�����

typedef struct
{
    uint8_t     Maximum;
    SplitType   Split[SplitMax];                //20
    uint8_t     Reserve[11];
}SplitTable;     //���űȱ� 65 * 20 + 12 = 1312 = 82 * 16 = 0x0520 


extern SplitType        SplitNow;
extern SplitTable       SplitTab;  //���űȱ� 
extern PhaseSplitType   RingSplit[RingMax];      //���ű�

uint8_t GetSplitPhaseIndex(SplitType* Split, uint8_t PhaseNum);

void SplitDefault(void); 
void SplitDataInit(uint8_t n);
void SplitXDataInit(SplitType* Split);


//coordPatternStatus    //2.5.10
//localFreeStatus       //2.5.11
//coordCycleStatus      //2.5.12
//coordSyncStatus       //2.5.13
//systemPatternControl  //2.5.14
//systemSyncControl     //2.5.15


#endif
