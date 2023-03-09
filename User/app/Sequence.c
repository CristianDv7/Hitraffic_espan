/*
*********************************************************************************************************
*
*	模块名称 : Módulo de secuencia de fase
*	文件名称 : Sequence.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Sequence.h"

//Datos de secuencia de fase del esquema actualmente en ejecución y el siguiente
uint8_t         RingStatus[RingMax];
SequenceType    SequenceNow;
SequenceTable   SeqTab;   //Tabla de secuencia de fases


/*
   Parámetro: puntero de secuencia de fase
    Función: juzgar cuántas fases tiene la secuencia de fases
          El número de timbre de la secuencia de fase no es válido y el número de fases se devuelve como 0
*/
uint8_t GetSeqMax(RingType* Ring)
{
    uint8_t i,phaseMax = 0;
    if(IsRing(Ring->RingNum))
    {
        for(i=0;i<PhaseMax;i++)
        {
            if(IsPhase(Ring->Phase[i]))
                phaseMax++;
            else
                break;
        }
    }

    return phaseMax;
}

void SequenceDefault(void)
{
    memset(&SeqTab.Maximum,0x00,sizeof(SeqTab));
    SeqTab.Maximum = SequenceMax;
    
    SeqTab.Seq[0].Num = 1;
    SeqTab.Seq[0].Ring[0].RingNum = 1;
    SeqTab.Seq[0].Ring[1].RingNum = 0;
    SeqTab.Seq[0].Ring[2].RingNum = 0;
    SeqTab.Seq[0].Ring[3].RingNum = 0;
    
    SeqTab.Seq[0].Ring[0].Phase[0] = 1;
    SeqTab.Seq[0].Ring[0].Phase[1] = 2;
    SeqTab.Seq[0].Ring[0].Phase[2] = 3;
    SeqTab.Seq[0].Ring[0].Phase[3] = 4;
    SeqTab.Seq[0].Ring[0].Phase[4] = 5;
    
    
    SeqTab.Seq[1].Num = 2;
    SeqTab.Seq[1].Ring[0].RingNum = 1;
    SeqTab.Seq[1].Ring[1].RingNum = 0;
    SeqTab.Seq[1].Ring[2].RingNum = 0;
    SeqTab.Seq[1].Ring[3].RingNum = 0;
    
    SeqTab.Seq[1].Ring[0].Phase[0] = 1;
    SeqTab.Seq[1].Ring[0].Phase[1] = 2;
    SeqTab.Seq[1].Ring[0].Phase[2] = 5;
}

//La configuración por defecto de la secuencia de fases Sequence
//Solo hay un anillo, y el anillo 1 libera 5 fases
void SequenceXDataInit(SequenceType* Sequence)
{
    Sequence->Num = 1;
    Sequence->Ring[0].RingNum = 1;//anillo 1 activo
    Sequence->Ring[1].RingNum = 0;//Solo 1 timbre, otros timbres no son válidos
    Sequence->Ring[2].RingNum = 0;
    Sequence->Ring[3].RingNum = 0;
    
    Sequence->Ring[0].Phase[0] = 1;
    Sequence->Ring[0].Phase[1] = 2;
    Sequence->Ring[0].Phase[2] = 3;
    Sequence->Ring[0].Phase[3] = 4;
    Sequence->Ring[0].Phase[4] = 5;
    Sequence->Ring[0].Phase[5] = 0;
}
