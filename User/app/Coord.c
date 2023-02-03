#include "public.h"
#include "Coord.h"

CoordType   Coord;

void CoordInit(void)
{
    memset(&Coord.OperationalMode,0x00,sizeof(Coord));
    Coord.OperationalMode = 0;
    Coord.CorrectionMode = 3;
    Coord.MaximumMode = 2;
    Coord.ForceMode = 2;
}

