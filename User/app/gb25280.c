#include "bsp.h" 				/* 底层硬件驱动 */
#include "gb25280.h"
#include "PhaseStateControl.h"

uint8_t gbtx[1536];
SEND_TYPE send;

uint8_t SearchResponse(GBMSG_TYPE *gbmsg, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    
    if(gbmsg->OperationType == OT_Query)//搜索操作
    {
        gbtx[0] = frame_head;
        gbtx[1] = frame_ver;
        gbtx[2] = ID_TSC;
        gbtx[3] = ID_PC;
        gbtx[4] = DL_Com;
        gbtx[5] = gbmsg->AreaNum;
        gbtx[6] = gbmsg->IntersectionL;
        gbtx[7] = gbmsg->IntersectionH;
        gbtx[8] = OT_QueryAnswer;
        gbtx[9] = O_Call; 
        gbtx[10] = Reserve_Data; 
        /* 发送信号机基本信息：型号，输出路数，支持功能， 等等*/
        gbtx[11] = DeviceInfo.DeviceVersion[0];
        gbtx[12] = DeviceInfo.DeviceVersion[1];
        gbtx[13] = DeviceInfo.DeviceVersion[2];
        gbtx[14] = DeviceInfo.DeviceVersion[3];
        gbtx[15] = ChannelMax * 3;
        gbtx[16] = DeviceInfo.DeviceNumber[14];
        gbtx[17] = DeviceInfo.DeviceNumber[15];
        
        CheckSumCalc = 0x00;
        for(i=1;i<=17;i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 18;
        if(CheckSumCalc == 0xC0)
        {
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else//不支持的操作类型
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t TimeOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_BasicInfo;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_CurrentTime;
    gbtx[10] = Reserve_Data;
                    
    if(gbmsg->OperationType == OT_Query)//查询时间
    {
        gbtx[8] = OT_QueryAnswer;
        
        gbtx[11] = DEC_to_BCD(Rtc.second);
        gbtx[12] = Rtc.minute;
        gbtx[13] = Rtc.hour;
        gbtx[14] = Rtc.week;
        gbtx[15] = Rtc.day;
        gbtx[16] = Rtc.month;
        gbtx[17] = Rtc.year;
        
        CheckSumCalc = 0x00;
        for(i=1;i<=17;i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 18;
        if(CheckSumCalc == 0xC0)
        {
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置时间
    {
        RtcType rtc;
        rtc.second  = pd[11];
        rtc.minute  = pd[12];
        rtc.hour    = pd[13];
        rtc.week    = pd[14];
        rtc.day     = pd[15];
        rtc.month   = pd[16];
        rtc.year    = pd[17];
        RtcWrite(&rtc);
        OP.TimeChangeFlag = 1;
        
        gbtx[8] = OT_SetAnswer;
        CheckSumCalc = 0x00;
        for(i=1;i<=10;i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else//不支持的操作类型
        msgerror = Unsupport_OT;

    return msgerror;
}

uint8_t ScheduleOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;

    uint8_t  *po;
    uint16_t ScheduleTabLength = sizeof(ScheduleType) * ScheduleMax + 1;
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Schedule;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询调度计划
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &ScheduleTab.Maximum;
        for(i = 0; i < ScheduleTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置调度计划
    {
        if(ScheduleMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &ScheduleTab.Maximum;
            for(i = 0; i < ScheduleTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            ScheduleWrite();
            OP.ScheduleDataChangeFlag = 1;

        
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t PlanOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;

    uint8_t  *po;
    uint16_t  PlanTabLength = sizeof(PlanType) * PlanMax + 1;//73*16 + 1 = 1169
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Plan;
    gbtx[10]= Reserve_Data;
                    
    if(gbmsg->OperationType == OT_Query)//查询日计划
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &PlanTab.Maximum;
        for(i = 0; i < PlanTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置日计划
    {
        if(PlanMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &PlanTab.Maximum;
            for(i = 0; i < PlanTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            PlanWrite();
            OP.PlanDataChangeFlag = 1;
            
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t ActionOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t  ActionTabLength = sizeof(ActionType) * ActionMax + 1; //4 * 100 + 1 = 401
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Action;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询动作
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &ActionTab.Maximum;
        for(i = 0; i < ActionTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置日计划
    {
        if(ActionMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &ActionTab.Maximum;
            for(i = 0; i < ActionTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            ActionWrite();
            OP.ActionDataChangeFlag = 1;

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t PatternOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t  PatternTabLength = sizeof(PatternType) * PatternMax + 1; //7 * 100 + 1 = 701
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Pattern;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询方案
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &PatternTab.Maximum;
        for(i = 0; i < PatternTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置方案
    {
        if(PatternMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &PatternTab.Maximum;
            for(i = 0; i < PatternTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            PatternWrite();
            OP.PatternDataChangeFlag = 1;

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t SequenceOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t  SeqTabLength = sizeof(SequenceType) * SequenceMax + 1;//69 * 16 + 1 = 1105
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Sequence;
    gbtx[10]= Reserve_Data;
                    
    if(gbmsg->OperationType == OT_Query)//查询相序
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &SeqTab.Maximum;
        for(i = 0; i < SeqTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置相序
    {
        if(SequenceMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &SeqTab.Maximum;
            for(i = 0; i < SeqTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            SequenceWrite();
            OP.SequenceDataChangeFlag = 1;
        
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t SplitOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t  SplitTabLength = sizeof(SplitType) * SplitMax + 1;//(4 * 16 + 1) * 20 + 1 = 341
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Split;
    gbtx[10]= Reserve_Data;
                    
    if(gbmsg->OperationType == OT_Query)//Consulta la proporción de letras verdes
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &SplitTab.Maximum;
        for(i = 0; i < SplitTabLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//Proporción de letra verde de diseño
    {
        if(SplitMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[11];
            po = &SplitTab.Maximum;
            for(i = 0; i < SplitTabLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            SplitWrite();
            #if DEBUG
            printf("Split Data Changed \r\n");
            #endif
            OP.SplitDataChangeFlag = 1;
            
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t PhaseOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t PhaseDataLength = sizeof(PhaseType) * PhaseMax;//32*16 = 512
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Phase;
    gbtx[10]= Reserve_Data;

    if(gbmsg->OperationType == OT_Query)//fase de consulta
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = PhaseMax;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &PhaseTab.Maximum + 1;
        for(i = 0; i < PhaseDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//establecer fase
    {
        if(PhaseMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[12];
            po = &PhaseTab.Maximum + 1;
            for(i = 0; i < PhaseDataLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            PhaseWrite();
            #if DEBUG
            printf("Phase Data Changed \r\n");
            #endif
            //OP.PhaseDataChangeFlag = 1;
            
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t ChannelOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t ChannelDataLength = sizeof(ChannelType) * ChannelMax;
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Channel;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询通道
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = ChannelMax;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &ChannelTab.Maximum + 1;
        for(i = 0; i < ChannelDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置通道
    {
        if(ChannelMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[12];
            po = &ChannelTab.Maximum + 1;
            for(i = 0; i < ChannelDataLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            ChannelWrite();
            #if DEBUG
            printf("Channel Data Changed \r\n");
            #endif
            
            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t UnitOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t UnitDataLength = 12;
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Unit;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询单元参数
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &Unit.StartupFlash;
        for(i = 0; i < UnitDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置单元参数
    {
        uint8_t *pi;
        pi = &pd[11];
        po = &Unit.StartupFlash;
        for(i = 0; i < UnitDataLength; i++)
        {
            if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
            {
                (*po) = 0xC0;
                pi += 2;
            }
            else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
            {
                (*po) = 0xDB;
                pi += 2;
            }
            else
            {
                (*po) = (*pi);
                pi++;
            }
            po++;
        }
        UnitWrite();
        #if DEBUG
        printf("Unit Data Changed \r\n");
        #endif

        gbtx[8] = OT_SetAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t CoordOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t CoordDataLength = 4;
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Coord;
    gbtx[10]= Reserve_Data;
                    
    if(gbmsg->OperationType == OT_Query)//查询协调参数
    {
        gbtx[8] = OT_QueryAnswer;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = &Coord.OperationalMode;
        for(i = 0; i < CoordDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置协调参数
    {
        uint8_t *pi;
        pi = &pd[11];
        po = &Coord.OperationalMode;
        for(i = 0; i < CoordDataLength; i++)
        {
            if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
            {
                (*po) = 0xC0;
                pi += 2;
            }
            else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
            {
                (*po) = 0xDB;
                pi += 2;
            }
            else
            {
                (*po) = (*pi);
                pi++;
            }
            po++;
        }
        CoordWrite();
        #if DEBUG
        printf("Coord Data Changed \r\n");
        #endif

        gbtx[8] = OT_SetAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t OverlapOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t OverlapDataLength = sizeof(OverlapType) * OverlapMax;
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_CharacPar;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Overlap;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询跟随相位
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = OverlapMax;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &OverlapTab.Maximum + 1;
        for(i = 0; i < OverlapDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置跟随相位
    {
        if(OverlapMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[12];
            po = &OverlapTab.Maximum + 1;
            for(i = 0; i < OverlapDataLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            OverlapWrite();
            for(i=0;i<OverlapMax;i++)
            {
                IncludedPhases[i] = OverlapTab.Overlap[i].IncludedPhases[0]|(OverlapTab.Overlap[i].IncludedPhases[1]<<8);     
                ModifierPhases[i] = OverlapTab.Overlap[i].ModifierPhases[0]|(OverlapTab.Overlap[i].ModifierPhases[1]<<8);;     //母相位 所跟随放行的机动车相位
            }
            #if DEBUG
            printf("Overlap Data Changed \r\n");
            #endif

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t RemoteControlOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;

    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_Intervention;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_RemoteControl;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Set)//设置
    {
        if(ClearError == pd[11])
        {
            #if DEBUG
            printf("Clear Error\r\n");
            #endif
            if(OP.WorkMode_Reason)
            {
                if(WMR_RedGreenConflict == OP.WorkMode_Reason)
                {
                    OP.red_green_conflict_reg = 0;//清楚故障
                    OP.WorkMode_Reason = WMR_Normal;//使信号机正常黄闪,而非故障黄闪
                }
                if(WMR_GreenConflict == OP.WorkMode_Reason)
                {
                    OP.green_conflict_reg = 0;//清楚故障
                    OP.WorkMode_Reason = WMR_Normal;//使信号机正常黄闪,而非故障黄闪
                }
                if(WMR_RedFailed == OP.WorkMode_Reason)
                {
                    OP.red_install_reg = red_install_fail_detect(0);//清楚故障
                    OP.WorkMode_Reason = WMR_Normal;//使信号机正常黄闪,而非故障黄闪
                }
            }
            
            gbtx[8] = OT_SetAnswer;
            gbtx[11]= ClearError;
            
            CheckSumCalc = 0;
            for(i = 1; i <= 11; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            
            num = 12;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
        else if(Manual_start == pd[11] || Manual_auto == pd[11])
        {
            uint8_t *po;
            //WorkPoint
            if(Manual_start == pd[11])//开始手动
            {
                #if DEBUG
                printf("Manual_start\r\n");
                #endif
                gbtx[8] = OT_SetAnswer;
                if(Manual_appoint == pd[12])
                {
                    ManualCtrl.Pos = pd[13];
                    ManualCtrl.Dir = pd[14];
                    ManualCtrl.AutoTime = pd[15]|(pd[16]<<8);
                    if(ManualCtrl.Pos == MANUAL_POS_Other || ManualCtrl.Dir == MANUAL_DIR_Other)
                    {
                        if(OP.WorkMode != ManualAppoint)
                        {
                            if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                            OP.WorkMode = ManualAppoint;
                            ManualCtrl.StartFlag = 1;
                            ManualCtrl.RemoteCtrlFlag = 1;
                            ManualCtrl.EnforceFlag = 0;
                            ManualCtrl.OrderFlag = 0;
                            ManualCtrl.ExitFlag = 0;
                            //ManualCtrl.Time = 0;
                        }
                        #if DEBUG
                        printf("ManualCtrlT_resident \r\n");
                        #endif
                    }
                    else 
                    {
                        if(OP.WorkMode != ManualAppoint)
                        {
                            if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                            OP.WorkMode = ManualAppoint;
                            ManualCtrl.StartFlag = 1;
                        }
                        
                        ManualCtrl.RemoteCtrlFlag = 1;
                        ManualCtrl.EnforceFlag = 1;
                        ManualCtrl.OrderFlag = 1;
                        ManualCtrl.ExitFlag = 0;
                        //ManualCtrl.Time = 0;
                        ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                        
                        #if DEBUG
                        printf("ManualCtrlT_Appoint\r\n");
                        printf("ChannelOnsNext = %04x\r\n", ManualCtrl.ChannelOnsNext);
                        printf("Pos = %02x\r\n", ManualCtrl.Pos);
                        printf("Dir = %02x\r\n", ManualCtrl.Dir);
                        #endif
                    }
                }
                else 
                {
                    ManualCtrl.RemoteCtrlFlag = 1;
                    ManualCtrl.AutoTime = pd[15]|(pd[16]<<8);
                    ManualCtrl.Time = ManualCtrl.AutoTime;
                    if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                    if(Manual_yellowflash == pd[12])
                    {
                        OP.WorkMode = ManualFlashing;
                    }
                    else if(Manual_allred == pd[12])
                    {
                        OP.WorkMode = ManualAllRead;
                    }
                    else if(Manual_lampoff == pd[12])
                    {
                        OP.WorkMode = ManualLampOff;
                    }
                    else if(Manual_nextphase == pd[12])
                    {
                        OP.WorkMode = ManualStep;
                        ManualCtrl.NextStepFlag = 1;
                    }
                }
                OP.WorkMode_Reason = WMR_RemoteManual;
            }
            else if(Manual_auto == pd[11])
            {
                if(OP.WorkMode == ManualAppoint)
                {
                    ManualCtrl.ExitFlag = 1;
                }
                else if(OP.WorkMode >= SPECIAL_MODE)
                {
                    ManualCtrl.LocalCtrlFlag = 0;
                    ManualCtrl.RemoteCtrlFlag = 0;
                    OP.WorkMode = OP.WorkModeBK;
                    OP.WorkMode_Reason = WMR_Normal;
                }
                #if DEBUG
                printf("ManualCtrlT_auto\r\n");
                #endif
            }

            num = 11;
            po = &pd[11];
            for(i = 0; i < 6; i++)
            {
                if((*po) == 0xC0)
                {
                    gbtx[num++] = 0xDB;
                    gbtx[num++] = 0xDC;
                }
                else if((*po) == 0xDB)
                {
                    gbtx[num++] = 0xDB;
                    gbtx[num++] = 0xDD;
                }
                else
                {
                    gbtx[num++] = (*po);
                }
                po++;
            }
            
            CheckSumCalc = 0;
            for(i = 1; i < num; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
        else msgerror = Frame_wrong;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t VehdetOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t DataLength = sizeof(VehdetTab);
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_Intervention;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Vehdet;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = VehdetMax;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &VehdetTab.vehdet[0].Num;
        for(i = 0; i < DataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置
    {
        if(VehdetMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[12];
            po = &VehdetTab.vehdet[0].Num;
            for(i = 0; i < DataLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            VehdetWrite();
            #if DEBUG
            printf("Peddet Data Changed \r\n");
            #endif

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t PeddetOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t PeddetDataLength = sizeof(Peddet) * PeddetMax;
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_Intervention;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_Peddet;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = PeddetMax;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &PeddetTab.Maximum + 1;
        for(i = 0; i < PeddetDataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置
    {
        if(PeddetMax != pd[11]) msgerror = DataError;
        else
        {
            uint8_t *pi;
            pi = &pd[12];
            po = &PeddetTab.Maximum + 1;
            for(i = 0; i < PeddetDataLength; i++)
            {
                if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
                {
                    (*po) = 0xC0;
                    pi += 2;
                }
                else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
                {
                    (*po) = 0xDB;
                    pi += 2;
                }
                else
                {
                    (*po) = (*pi);
                    pi++;
                }
                po++;
            }
            PeddetWrite();
            #if DEBUG
            printf("Peddet Data Changed \r\n");
            #endif

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t PeddetStateOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t PeddetStateLength = sizeof(PeddetState) * PeddetMax;

    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_Intervention;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_PeddetState;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询
    {
        gbtx[8] = OT_QueryAnswer;
        gbtx[11] = PeddetMax;
        
        #if DEBUG
        printf("sizeof(PeddetState) = %d", sizeof(PeddetState));
        #endif
        
        CheckSumCalc = 0;
        for(i = 1; i <= 11; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 12;
        
        po = &PeddetStateTab.peddet[0].Valid;
        for(i = 0; i < PeddetStateLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置
    {
        if(PeddetMax != pd[11]) msgerror = DataError;
        else
        {
            PeddetStateInit();

            #if DEBUG
            printf("PeddetState Cleared \r\n");
            #endif

            gbtx[8] = OT_SetAnswer;
            CheckSumCalc = 0;
            for(i = 1; i <= 10; i++)
            {
                CheckSumCalc += gbtx[i];
            }
            num = 11;
            if(CheckSumCalc == 0xC0){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDC;
            }else if(CheckSumCalc == 0xDB){
                gbtx[num++]= 0xDB;
                gbtx[num++]= 0xDD;
            }else{
                gbtx[num++]= CheckSumCalc;
            }
            gbtx[num++]= frame_tail;
            send->n = 1;
            send->length0 = num;
            send->pdata0 = gbtx;
            msgerror = Frame_right;
        }
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t WorkStateOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t DataLength;
    uint8_t temp[32];
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_BasicInfo;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_WorkState;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询状态信息
    {
        gbtx[8] = OT_QueryAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        //时段号，环号，
        temp[0] = ScheduleNow.Num; //调度计划号
        temp[1] = Plan.Num;     //时段表号
        temp[2] = Action.Num;   //动作号
        temp[3] = PatternNow.Num;  //方案号
        temp[4] = SequenceNow.Num; //相序表
        temp[5] = SplitNow.Num;    //绿信比
        
        temp[6] = OP.WorkMode;
        temp[7] = OP.WorkMode_Reason;
        if(OP.WorkMode_Reason == WMR_RedFailed)
        {
            temp[8] = OP.red_failed_reg&0xff;
            temp[9] =(OP.red_failed_reg>>8);
        }
        else if(OP.WorkMode_Reason == WMR_RedGreenConflict)
        {
            temp[8] = OP.red_green_conflict_reg&0xff;
            temp[9] =(OP.red_green_conflict_reg>>8);
        }
        else if(OP.WorkMode_Reason == WMR_GreenConflict)
        {
            temp[8] = OP.green_conflict_reg&0xff;
            temp[9] =(OP.green_conflict_reg>>8);
        }
        else
        {
            temp[8] = 0;
            temp[9] = 0;
        }
        
        temp[10] = OP.Reg1sCount&0xff;
        temp[11] = OP.Reg1sCount>>8;
        temp[12] = OP.Reg1sCount>>16;
        temp[13] = OP.Reg1sCount>>24;
        
        temp[14] = PeddetStateTab.peddet[7].Valid;
        i = 15;
        temp[i++] = SequenceNow.Ring[0].Phase[PhaseState.Ring[0].SeqNum];//相位号
        temp[i++] = RingSplit[0].Time;//相位时间
        temp[i++] = PhaseState.Ring[0].SecondRemain;//剩余时间
        temp[i++] = SequenceNow.Ring[1].Phase[PhaseState.Ring[1].SeqNum];//相位号
        temp[i++] = RingSplit[1].Time;//相位时间
        temp[i++] = PhaseState.Ring[1].SecondRemain;//剩余时间
        temp[i++] = SequenceNow.Ring[2].Phase[PhaseState.Ring[2].SeqNum];//相位号
        temp[i++] = RingSplit[2].Time;//相位时间
        temp[i++] = PhaseState.Ring[2].SecondRemain;//剩余时间
        temp[i++] = SequenceNow.Ring[3].Phase[PhaseState.Ring[3].SeqNum];//相位号
        temp[i++] = RingSplit[3].Time;//相位时间
        temp[i++] = PhaseState.Ring[3].SecondRemain;//剩余时间
        
        DataLength = 27;
        po = temp;
        for(i = 0; i < DataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

//27+12 = 39
uint8_t SignalState(uint8_t* temp)
{
    uint8_t i = 0;
    uint32_t Statustemp = 0;
    
    //时段号，环号，10 byte
    temp[i++] = ScheduleNow.Num; //调度计划号
    temp[i++] = Plan.Num;     //时段表号
    temp[i++] = Action.Num;   //动作号
    temp[i++] = PatternNow.Num;  //方案号
    temp[i++] = SequenceNow.Num; //相序表
    temp[i++] = SplitNow.Num;    //绿信比

    temp[i++] = OP.WorkMode;
    temp[i++] = OP.WorkMode_Reason;
    if(OP.WorkMode_Reason == WMR_RedFailed)
    {
        temp[i++] = OP.red_failed_reg&0xff;
        temp[i++] =(OP.red_failed_reg>>8);
    }
    else if(OP.WorkMode_Reason == WMR_RedGreenConflict)
    {
        temp[i++] = OP.red_green_conflict_reg&0xff;
        temp[i++] =(OP.red_green_conflict_reg>>8);
    }
    else if(OP.WorkMode_Reason == WMR_GreenConflict)
    {
        temp[i++] = OP.green_conflict_reg&0xff;
        temp[i++] =(OP.green_conflict_reg>>8);
    }
    else if(OP.WorkMode > SPECIAL_MODE)
    {
        temp[i++] = ManualCtrl.Time&0xff;
        temp[i++] = ManualCtrl.Time >> 8;
    }
    else
    {
        temp[i++] = 0;
        temp[i++] = 0;
    }
    
    //12 byte 环相位数据
    temp[i++] = SequenceNow.Ring[0].Phase[PhaseState.Ring[0].SeqNum];//相位号
    temp[i++] = RingSplit[0].Time;//相位时间
    temp[i++] = PhaseState.Ring[0].SecondRemain;//剩余时间
    temp[i++] = SequenceNow.Ring[1].Phase[PhaseState.Ring[1].SeqNum];//相位号
    temp[i++] = RingSplit[1].Time;//相位时间
    temp[i++] = PhaseState.Ring[1].SecondRemain;//剩余时间
    temp[i++] = SequenceNow.Ring[2].Phase[PhaseState.Ring[2].SeqNum];//相位号
    temp[i++] = RingSplit[2].Time;//相位时间
    temp[i++] = PhaseState.Ring[2].SecondRemain;//剩余时间
    temp[i++] = SequenceNow.Ring[3].Phase[PhaseState.Ring[3].SeqNum];//相位号
    temp[i++] = RingSplit[3].Time;//相位时间
    temp[i++] = PhaseState.Ring[3].SecondRemain;//剩余时间
    

    
    //12 byte 通道状态
    if(PhaseState.Phase10msCount < 50) Statustemp = (~(ChannelStatus.Reds&ChannelStatus.Flash))&ChannelStatus.Reds;
    else Statustemp = ChannelStatus.Reds;
    temp[i++] = (Statustemp>>24);
    temp[i++] = (Statustemp>>16);
    temp[i++] = (Statustemp>>8);
    temp[i++] = (Statustemp&0xff);
    if(PhaseState.Phase10msCount < 50) Statustemp = (~(ChannelStatus.Yellows&ChannelStatus.Flash))&ChannelStatus.Yellows;
    else Statustemp = ChannelStatus.Yellows;
    temp[i++] = (Statustemp>>24);
    temp[i++] = (Statustemp>>16);
    temp[i++] = (Statustemp>>8);
    temp[i++] = (Statustemp&0xff);
    if(PhaseState.Phase10msCount < 50) Statustemp = (~(ChannelStatus.Greens&ChannelStatus.Flash))&ChannelStatus.Greens;
    else Statustemp = ChannelStatus.Greens;
    temp[i++] = (Statustemp>>24);
    temp[i++] = (Statustemp>>16);
    temp[i++] = (Statustemp>>8);
    temp[i++] = (Statustemp&0xff);
    
    //系统持续运行时间
    temp[i++] = OP.Reg1sCount&0xff;
    temp[i++] = OP.Reg1sCount>>8;
    temp[i++] = OP.Reg1sCount>>16;
    temp[i++] = OP.Reg1sCount>>24;
    //行人检测器8作为开门状态标志
    temp[i++] = PeddetStateTab.peddet[7].Valid;
    return i;
}

uint8_t SignalStateOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t DataLength;
    uint8_t temp[64];
    
    if(gbmsg->OperationType == OT_Query)//查询状态信息
    {
        if(gbmsg->Reserve == 0x02) OP.SendWorkModeAutoFlag = 1; else OP.SendWorkModeAutoFlag = 0;
        
        gbtx[0] = frame_head;
        gbtx[1] = frame_ver;
        gbtx[2] = ID_TSC;
        gbtx[3] = ID_PC;
        gbtx[4] = DL_BasicInfo;
        gbtx[5] = gbmsg->AreaNum;
        gbtx[6] = gbmsg->IntersectionL;
        gbtx[7] = gbmsg->IntersectionH;
        gbtx[8] = OT_QueryAnswer;
        gbtx[9] = O_WorkState;
        gbtx[10]= gbmsg->Reserve;
        
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        
        DataLength = SignalState(temp);
        num = 11;
        po = temp;
        for(i = 0; i < DataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

void SignalStateAutoReport(SEND_TYPE* send)
{
    uint16_t i,num,DataLength;
    uint8_t  CheckSumCalc;
    uint8_t  *po;
    uint8_t temp[64];
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_BasicInfo;
    gbtx[5] = 0X01;
    gbtx[6] = 0X01;
    gbtx[7] = 0X00;
    gbtx[8] = OT_InitiativeReport;
    gbtx[9] = O_WorkState;
    if(OP.SendWorkModeAutoFlag) gbtx[10]= 0x02; else gbtx[10]= Reserve_Data;
    
    CheckSumCalc = 0;
    for(i = 1; i <= 10; i++)
    {
        CheckSumCalc += gbtx[i];
    }
    
    DataLength = SignalState(temp);
    num = 11;
    po = temp;
    for(i = 0; i < DataLength; i++)
    {
        if((*po) == 0xC0)
        {
            gbtx[num++] = 0xDB;
            gbtx[num++] = 0xDC;
            CheckSumCalc += 0xDB + 0xDC;
        }
        else if((*po) == 0xDB)
        {
            gbtx[num++] = 0xDB;
            gbtx[num++] = 0xDD;
            CheckSumCalc += 0xDB + 0xDD;
        }
        else
        {
            gbtx[num++] = (*po);
            CheckSumCalc += (*po);
        }
        po++;
    }
    
    if(CheckSumCalc == 0xC0){
        gbtx[num++]= 0xDB;
        gbtx[num++]= 0xDC;
    }else if(CheckSumCalc == 0xDB){
        gbtx[num++]= 0xDB;
        gbtx[num++]= 0xDD;
    }else{
        gbtx[num++]= CheckSumCalc;
    }
    gbtx[num++]= frame_tail;
    send->n = 1;
    send->length0 = num;
    send->pdata0 = gbtx;
}

void DoorAlarmReport(SEND_TYPE* send)
{
    uint16_t i,num,DataLength;
    uint8_t  CheckSumCalc;
    uint8_t  *po;
    uint8_t temp[16];
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_Intervention;
    gbtx[5] = 0X01;
    gbtx[6] = 0X01;
    gbtx[7] = 0X00;
    gbtx[8] = OT_InitiativeReport;
    gbtx[9] = O_Alarm;
    gbtx[10]= Reserve_Data;
    
    CheckSumCalc = 0;
    for(i = 1; i <= 10; i++)
    {
        CheckSumCalc += gbtx[i];
    }
    num = 11;

    temp[0] = TypeAlarm_Environment;
    temp[1] = DoorState;
    temp[2] = PeddetStateTab.peddet[7].Valid;
    temp[3] = 0x00;
    temp[4] = 0x00;
    temp[5] = 0x00;
    temp[6] = 0x00;
    temp[7] = 0x00;

    DataLength = 8;
    po = temp;
    for(i = 0; i < DataLength; i++)
    {
        if((*po) == 0xC0)
        {
            gbtx[num++] = 0xDB;
            gbtx[num++] = 0xDC;
            CheckSumCalc += 0xDB + 0xDC;
        }
        else if((*po) == 0xDB)
        {
            gbtx[num++] = 0xDB;
            gbtx[num++] = 0xDD;
            CheckSumCalc += 0xDB + 0xDD;
        }
        else
        {
            gbtx[num++] = (*po);
            CheckSumCalc += (*po);
        }
        po++;
    }
    
    if(CheckSumCalc == 0xC0){
        gbtx[num++]= 0xDB;
        gbtx[num++]= 0xDC;
    }else if(CheckSumCalc == 0xDB){
        gbtx[num++]= 0xDB;
        gbtx[num++]= 0xDD;
    }else{
        gbtx[num++]= CheckSumCalc;
    }
    gbtx[num++]= frame_tail;
    send->n = 1;
    send->length0 = num;
    send->pdata0 = gbtx;
}

uint8_t BasicInfoOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t DataLength = sizeof(BasicInfo);
    //printf("DataLength = %d\r\n", DataLength);
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_BasicInfo;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_BasicInfo;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询基础信息
    {
        gbtx[8] = OT_QueryAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = BasicInfo.IntersectionInfo;
        for(i = 0; i < DataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置设备信息
    {
        uint8_t *pi;
        pi = &pd[11];
        po = BasicInfo.IntersectionInfo;
        for(i = 0; i < DataLength; i++)
        {
            if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
            {
                (*po) = 0xC0;
                pi += 2;
            }
            else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
            {
                (*po) = 0xDB;
                pi += 2;
            }
            else
            {
                (*po) = (*pi);
                pi++;
            }
            po++;
        }
        
        BasicInfoWrite();
        #if DEBUG
        printf("BasicInfo Changed \r\n");
        #endif
        
        gbtx[8] = OT_SetAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t DeviceInfoOperate(GBMSG_TYPE *gbmsg, uint8_t *pd, SEND_TYPE* send)
{
    uint16_t i,num;
    uint8_t  CheckSumCalc;
    uint8_t  msgerror;
    uint8_t  *po;
    uint16_t DataLength = sizeof(DeviceInfo);
    
    gbtx[0] = frame_head;
    gbtx[1] = frame_ver;
    gbtx[2] = ID_TSC;
    gbtx[3] = ID_PC;
    gbtx[4] = DL_BasicInfo;
    gbtx[5] = gbmsg->AreaNum;
    gbtx[6] = gbmsg->IntersectionL;
    gbtx[7] = gbmsg->IntersectionH;
    gbtx[9] = O_DeviceInfo;
    gbtx[10]= Reserve_Data;
    
    if(gbmsg->OperationType == OT_Query)//查询设备信息
    {
        gbtx[8] = OT_QueryAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        
        po = DeviceInfo.ManufacturerInfo;
        for(i = 0; i < DataLength; i++)
        {
            if((*po) == 0xC0)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDC;
                CheckSumCalc += 0xDB + 0xDC;
            }
            else if((*po) == 0xDB)
            {
                gbtx[num++] = 0xDB;
                gbtx[num++] = 0xDD;
                CheckSumCalc += 0xDB + 0xDD;
            }
            else
            {
                gbtx[num++] = (*po);
                CheckSumCalc += (*po);
            }
            po++;
        }
        
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else if(gbmsg->OperationType == OT_Set)//设置设备信息
    {
        uint8_t *pi;
        pi = &pd[11];
        po = DeviceInfo.ManufacturerInfo;
        for(i = 0; i < DataLength; i++)
        {
            if((*pi) == 0xDB && (*(pi+1)) == 0xDC)
            {
                (*po) = 0xC0;
                pi += 2;
            }
            else if((*pi) == 0xDB && (*(pi+1)) == 0xDD)
            {
                (*po) = 0xDB;
                pi += 2;
            }
            else
            {
                (*po) = (*pi);
                pi++;
            }
            po++;
        }
        
        DeviceInfoWrite();
        #if DEBUG
        printf("DeviceInfo Changed \r\n");
        #endif
        
        gbtx[8] = OT_SetAnswer;
        CheckSumCalc = 0;
        for(i = 1; i <= 10; i++)
        {
            CheckSumCalc += gbtx[i];
        }
        num = 11;
        if(CheckSumCalc == 0xC0){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDC;
        }else if(CheckSumCalc == 0xDB){
            gbtx[num++]= 0xDB;
            gbtx[num++]= 0xDD;
        }else{
            gbtx[num++]= CheckSumCalc;
        }
        gbtx[num++]= frame_tail;
        send->n = 1;
        send->length0 = num;
        send->pdata0 = gbtx;
        msgerror = Frame_right;
    }
    else
        msgerror = Unsupport_OT;
    
    return msgerror;
}

uint8_t gb25280_Process(uint8_t *pd, uint16_t size, SEND_TYPE* send)
{
    uint8_t msgerror;
    uint8_t CheckSumReceive,CheckSumCalc = 0;
    uint16_t i;
    GBMSG_TYPE *gbmsg;
    gbmsg = (GBMSG_TYPE *)pd;
    
    if(gbmsg->Head != frame_head || gbmsg->Version != frame_ver || gbmsg->Sendid!= ID_PC || gbmsg->Reciveid != ID_TSC || pd[size-1] != frame_tail) return Frame_wrong;

    if(pd[size-3] == 0xDB && pd[size-2] == 0xDC) 
    {
        CheckSumReceive = 0xC0;
        for(i=1;i<size-3;i++)
        {
            CheckSumCalc += pd[i];
        }
    }
    else if(pd[size-3] == 0xDB && pd[size-2] == 0xDD) 
    {
        CheckSumReceive = 0xDB;
        for(i=1;i<size-3;i++)
        {
            CheckSumCalc += pd[i];
        }
    }
    else
    {
        CheckSumReceive = pd[size-2];
        for(i=1;i<size-2;i++)
        {
            CheckSumCalc += pd[i];
        }
    }
    if(CheckSumCalc != CheckSumReceive) return CheckSum_failed;//校验错误
    
    if(gbmsg->DataLinkCode == DL_BasicInfo)//基本信息
    {
        if(gbmsg->ObjectCode == O_CurrentTime)//时间操作
        {
            msgerror = TimeOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_DeviceInfo)//基本信息操作
        {
            msgerror = DeviceInfoOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_BasicInfo)//基本信息操作
        {
            msgerror = BasicInfoOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_WorkState)//基本信息操作
        {
            msgerror = SignalStateOperate(gbmsg, pd, send);
        }
        else
        {
            msgerror = Unsupport_OB;
        }
    }
    else if(gbmsg->DataLinkCode == DL_Com)//通信链路
    {
        if(gbmsg->ObjectCode == O_Call)//搜索操作
        {
            msgerror = SearchResponse(gbmsg, send);
        }
    }
    else if(gbmsg->DataLinkCode == DL_CharacPar)
    {
        switch(gbmsg->ObjectCode)
        {
            case O_Schedule:    msgerror = ScheduleOperate(gbmsg, pd, send);    break;  //调度计划操作
            case O_Plan:        msgerror = PlanOperate(gbmsg, pd, send);        break;  //日计划操作
            case O_Action:      msgerror = ActionOperate(gbmsg, pd, send);      break;  //动作操作
            case O_Pattern:     msgerror = PatternOperate(gbmsg, pd, send);     break;  //方案操作
            case O_Sequence:    msgerror = SequenceOperate(gbmsg, pd, send);    break;  //相序操作
            case O_Split:       msgerror = SplitOperate(gbmsg, pd, send);       break;  //绿信比操作
            case O_Phase:       msgerror = PhaseOperate(gbmsg, pd, send);       break;  //相位操作
            case O_Channel:     msgerror = ChannelOperate(gbmsg, pd, send);     break;  //通道操作
            case O_Unit:        msgerror = UnitOperate(gbmsg, pd, send);        break;  //单元参数操作
            case O_Coord:       msgerror = CoordOperate(gbmsg, pd, send);       break;  //协调操作
            case O_Overlap:     msgerror = OverlapOperate(gbmsg, pd, send);     break;  //跟随相位操作
            default:            msgerror = Unsupport_OB;                        break;
        }
    }
    else if(gbmsg->DataLinkCode == DL_Intervention)
    {
        if(gbmsg->ObjectCode == O_Vehdet)//行人检测器操作
        {
            msgerror = VehdetOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_Peddet)//行人检测器操作
        {
            msgerror = PeddetOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_PeddetState)
        {
            msgerror = PeddetStateOperate(gbmsg, pd, send);
        }
        else if(gbmsg->ObjectCode == O_RemoteControl)
        {
            msgerror = RemoteControlOperate(gbmsg, pd, send);
        }
        else msgerror = Unsupport_OB;
    }
    else 
        msgerror = Unsupport_DL;
        
    return msgerror;
}
