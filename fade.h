void SleepQ(int i)
{ 
   int x, y; 
   int c; 
   for (y = 0; y < i; y++) 
   { 
      for (x = 0; x < 4000; x++) 
         c = c + 2; // do something to slow things down 
   } 
} 


//****************************************************************************** 
void FadeOut( u32 aWait ) 
{ 
   s8 Phase; 
   REG_BLDMOD = 0 | 1 | 2 | 4 | 8 | 128 | 64 | 32; 
   for( Phase = 0; Phase < 17; Phase++ ) 
   { 
      REG_COLEY = Phase; 
      SleepQ( aWait ); 
   } 
} 

//****************************************************************************** 
void FadeIn( u32 aWait ) 
{ 
   s8 Phase; 
   REG_BLDMOD = 0 | 1 | 2 | 4 | 8 | 128 | 64 | 32; 
   for( Phase = 0; Phase < 16; Phase++ ) 
   { 
      REG_COLEY = 16-Phase; 
      SleepQ ( aWait ); 
   } 
}
