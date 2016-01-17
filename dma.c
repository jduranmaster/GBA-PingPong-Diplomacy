#include "gba.h"
#include "dma.h"

/*************************************************************************************************************\
|                                                                                                             |
|  DMACopy                                                                                                   |
|                                                                                                             |
|*************************************************************************************************************|
|  Performs a fast DMA transfer of data                                                                       |
|*************************************************************************************************************|
|  u8 nChannel    - DMA Channel to use                                                                        |
|  void *pSource  - Pointer to memory to be transferred                                                       |
|  void *pDest    - Pointer to location to transfer to                                                        |
|  u32 nWordCount - Number of words (16 or 32 bits) to transfer                                               |
|  u32 nMode      - Transfer mode, such as DMA_32, DMA_TIMING_IMMEDIATE etc.                                  |
|*************************************************************************************************************|
|  No return value                                                                                            |
\*************************************************************************************************************/
void DMACopy(u8 nChannel, void *pSource, void *pDest, u32 nWordCount, u32 nMode)
{
	switch (nChannel)
	{
		case 0: 
			REG_DM0SAD = (u32)pSource;
			REG_DM0DAD = (u32)pDest;			
			REG_DM0CNT = nWordCount | nMode;
			break;
		case 1:
			REG_DM1SAD = (u32)pSource;
			REG_DM1DAD = (u32)pDest;
			REG_DM1CNT = nWordCount | nMode;
			break;
		case 2:
			REG_DM2SAD = (u32)pSource;
			REG_DM2DAD = (u32)pDest;
			REG_DM2CNT = nWordCount | nMode;
			break;
		case 3:
			REG_DM3SAD = (u32)pSource;
			REG_DM3DAD = (u32)pDest;
			REG_DM3CNT = nWordCount | nMode;
			break;
	}
}
