/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program orig. <  2 KByte (of non-blank source), by H.G. Muller  */
/***************************************************************************/
/* http://home.hccnet.nl/h.g.muller/umax4_8.c                              */
/* version 4.8.j6  ported and reworked for RasPi by dsyleixa               */
/* features:                                                               */
/* version 4.8  features:                                                  */
/* - recursive negamax search                                              */
/* - all-capture MVV/LVA quiescence search                                 */
/* - (internal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - a hash table storing score and best move                              */
/* - futility pruning                                                      */
/* - king safety through magnetic, frozen king in middle-game              */
/* - R=2 null-move pruning                                                 */
/* - keep hash and repetition-draw detection                               */
/* - better defense against passers through gradual promotion              */
/* - extend check evasions in inner nodes                                  */
/* - reduction of all non-Pawn, non-capture moves except hash move (LMR)   */
/* - full FIDE rules (expt under-promotion) and move-legality checking     */
 


// Code für ESP32

float  movecount=1.0;



#define HASHSIZE (1<<10) //  wegen RAM, für PC: (1<<24) // <<<<<<  für Arduino Mega 1<<8 !



struct HTab {
          int  Key,
               Score;
          int  From,
               To,
               Draft;
        } HashTab[HASHSIZE];                             // hash table, HTsize entries 



#define MAXNODES  120000L   // max deepening: increased values => higher skills

int  K,
     RootEval,
     R,
     HashKeyLo,
     HashKeyHi;

int32_t   N,
          INF=8000;                                     // INF=8000: "infinity eval score"   ;

int
    bchk=136,                                            // board check: bchk=0x88   board system     
    S=128,                                               // dummy square 0x80, highest valid square =127     
    Side=16;                                             // 16=white, 8=black; Side^=24 switches ther and back

signed char  L;
int    busycount=0;

//------------------------------------------------------------

                         // piece list (number, type) is {1,2,3,4,5,6,7} = {P+,P-,N,K,B,R,Q}. 
                         // Type 0 is not used, so it can indicate empty squares on the board.
signed char  
     pval[]={0,2,2,7,-1,8,12,23},                        // relative piece values     
                                        // step-vect list:
                                        // piece type p finds starting dir[j] at [p+16]
     pvect[]={-16,-15,-17,0,                             // pawns
             1,16,0,                                    // rook
              1,16,15,17,0,                              // king, queen and bishop
             14,18,31,33,0,                             // knight   
              7,-1,11,6,8,3,6},                          // 1st dir. in pvect[] per piece 

     stdBaseLn[]={6,3,5,7,4,5,3,6},     // initial piece setup  
          
     board[129],                                         // board: half of 16x8+dummy 
     T[1035];                                            // hash translation table    

signed char  psymbol[]= ".?+nkbrq?*?NKBRQ";              // .empty ?undef +downstreamPawn *upstreamPawn
                                                         // nKnight kKing bBishop rRook qQueen 

int  mfrom, mto;       // current ply from - to
int  Rootep,           // e.PieceType. square
     rmvP=128;         // remove piece
     


//------------------------------------------------------------
// compute HashTable value
// #define Kb(A,B) *(int*)(T+A+(B&8)+S*(B&7)) 
static int  Kb( const signed char A, const int B )
{
   void const * const ptr = T+A+(B&8)+S*(B&7);
   int result;
   memcpy(&result, ptr, 4);
   return result;
}
 

//------------------------------------------------------------
// recursive minimax search, Side=moving side, n=depth 
//------------------------------------------------------------
int  Minimax(int32_t  Alpha, int32_t  Beta, int32_t  Eval, int  epSqr, int  prev, int32_t   Depth)
                       // (Alpha,Beta)=window, Eval, EnPass_sqr. 
                       // prev=prev.dest; HashKeyLo,HashKeyHi=hashkeys; return EvalScore 
{
   int  j,
        StepVec,
        BestScore,
        v,
        IterDepth,
        h,
        i,
        SkipSqr,
        RookSqr,
        V,
        P,            // Null Move Prawning
        f=HashKeyLo,
        g=HashKeyHi,
        C,
        s;
   signed char  Victim,
        PieceType,
        Piece,
        FromSqr ,
        ToSqr ,
        BestFrom,
        BestTo,
        CaptSqr,
        StartSqr ;

   struct HTab *a = HashTab + ((HashKeyLo + Side * epSqr) & (HASHSIZE-1)); // lookup pos. in hash table, improved
   char   sbuf[100];
   
   Alpha--;                                             // adj. window: delay bonus  
   Side^=24;                                            // change sides              
   IterDepth  = a->Draft;
   BestScore  = a->Score;
   BestFrom   = a->From;
   BestTo     = a->To;                                  // resume at stored depth    

   if(a->Key-HashKeyHi | prev |                         // miss: other pos. or empty 
      !(BestScore<=Alpha | BestFrom&8&&BestScore>=Beta | BestFrom&S))            //   or window incompatible  
      { IterDepth=BestTo=0; }                           // start iter. from scratch  

   BestFrom&=~bchk;                                        // start at best-move hint   

   while( IterDepth++ <  Depth || IterDepth<3           // iterative deepening loop  
     || prev&K == INF
       && ( (N<MAXNODES  && IterDepth<98)                   // root: deepen upto time; changed from N<60000    
          || (K=BestFrom, L=BestTo&~bchk, IterDepth=3)
       )
     )                                                  // time's up: go do best     
   {
      FromSqr =StartSqr =BestFrom;                      // start scan at prev. best  
      h=BestTo&S;                                       // request try noncastl. 1st 
      P=IterDepth<3 ? INF : Minimax(-Beta,1-Beta,-Eval,S,0,IterDepth-3);            // Search null move          
      BestScore = (-P<Beta | R>35) ? ( IterDepth>2 ? -INF : Eval ) : -P;            // Prune or stand-pat        
      N++;                                              // node count (for timing)   
      do
      {
         Piece=board[FromSqr];                         // scan board looking for    
         if(Piece & Side)                               //  own piece (inefficient!) 
         {
            StepVec = PieceType = Piece&7;              // PieceType = piece type (set StepVec>0)  
            j = pvect[PieceType+16];                    // first step pvect f.piece 
            while(StepVec = PieceType>2 & StepVec<0 ? -StepVec : -pvect[++j] )       // loop over directions pvect[]  
            {
labelA:                                                                 // resume normal after best  
               ToSqr =FromSqr ;                                         // (FromSqr ,ToSqr )=move          
               SkipSqr= RookSqr =S;                                     // (SkipSqr, RookSqr )=castl.R       

               do
               {                                                        // ToSqr  traverses ray, or:      
                  CaptSqr=ToSqr =h?BestTo^h:ToSqr +StepVec;             // sneak in prev. best move  

                  if(ToSqr &bchk)break;                                 // board edge hit            

                  BestScore= epSqr-S&board[epSqr]&&ToSqr -epSqr<2&epSqr-ToSqr <2?INF:BestScore;      // bad castling              
                  
                  if( PieceType<3 && ToSqr==epSqr) CaptSqr^=16;         // CaptSqr for E.P. if Eval of PieceType=Pawn. 
                                                                        // <3 is a piecetype which represent pawns (1,2) 
                                                                        // (officers are >=3: 6,3,5,7,4)
                  Victim =board[CaptSqr];

                  if(Victim & Side|PieceType<3 & !(ToSqr -FromSqr & 7)-!Victim )break;            // capt. own, bad pawn mode  
                  i=37*pval[Victim & 7]+(Victim & 192);                 // value of capt. piece Victim     
                  BestScore =i<0?INF:BestScore ;                        // K capture                 

                  if(BestScore >=Beta & IterDepth>1) goto labelC;       // abort on fail high        

                  v=IterDepth-1?Eval:i-PieceType;                       // MVV/LVA scoring           

                  if(IterDepth-!Victim >1)                              // remaining depth           
                  {   
                     v=PieceType<6?board[FromSqr+8]-board[ToSqr+8]:0;            // center positional pts.    
                     board[RookSqr]=board[CaptSqr]=board[FromSqr]=0;board[ToSqr]=Piece|32;             // do move, set non-virgin   
                     if(!( RookSqr & bchk))board[SkipSqr]=Side+6,v+=50;               // castling: put R & Eval   
                     v-=PieceType-4|R>29?0:20;                                     // penalize mid-game K move  

                     if(PieceType<3)                                               // pawns:                    
                     {
                        v-=9*((FromSqr -2 & bchk||board[FromSqr -2]-Piece)+           // structure, undefended     
                               (FromSqr +2 & bchk||board[FromSqr +2]-Piece)-1         //        squares plus bias  
                              +(board[FromSqr ^16]==Side+36))                      // kling to non-virgin King  
                              -(R>>2);                                             // end-game Pawn-push bonus  
                         V=ToSqr +StepVec+1 & S?647-PieceType:2*(Piece & ToSqr +16 & 32);           // promotion or 6/7th bonus  
                         board[ToSqr]+=V;
                         i+=V;                                                     // change piece, add Eval   
                     }

                     v+= Eval+i;
                     V=BestScore >Alpha ? BestScore  : Alpha;                      // new eval and alpha        
                     HashKeyLo+=Kb(ToSqr +0,board[ToSqr])-Kb(FromSqr +0,Piece)-Kb(CaptSqr +0,Victim );
                     HashKeyHi+=Kb(ToSqr+8,board[ToSqr])-Kb(FromSqr+8,Piece)-Kb(CaptSqr+8,Victim )+ RookSqr -S;  // update hash key           
                     C = IterDepth-1-(IterDepth>5 & PieceType>2 & !Victim & !h);
                     C = R>29|IterDepth<3|P-INF?C:IterDepth;                       // extend 1 ply if in check  
                     do {
                        s= (C>2)||(v>V)? 
                             -Minimax(-Beta,-V,-v, SkipSqr,0,C)              // recursive eval. of reply                                                                                    
                              :v;                                            // or fail low if futile     
                     } while( s>Alpha && ++C<IterDepth );

                     v=s;
                     if(prev && K-INF && v+INF && (FromSqr==K) & (ToSqr==L) )      // move pending & in root:   
                     {
                        RootEval=-Eval-i; Rootep=SkipSqr;               // exit if legal & found   
                        a->Draft=99;a->Score=0;                         // lock game in hash as draw 
                        R+=i>>7;
                        return Beta;                                    // captured non-P material   
                     }
                     HashKeyLo=f;
                     HashKeyHi=g;                                       // restore hash key          
                     board[RookSqr]=Side+6;
                     board[SkipSqr]=board[ToSqr]=0;
                     board[FromSqr]=Piece;
                     board[CaptSqr]=Victim ;                           // undo move, RookSqr can be dummy  
                  }
                  if(v>BestScore )                                      // new best, update max,best 
                  {
                     BestScore = v, BestFrom=FromSqr, BestTo=(ToSqr | (S & SkipSqr));   // mark double move with S   
                  }
                  if(h)
                  {
                     h=0;
                     goto labelA;                                       // redo after doing old best 
                  }
                  if (
                    FromSqr + StepVec - ToSqr  || Piece & 32 ||               // not 1st step,moved before 
                    PieceType>2 && (PieceType-4 | j-7 ||                      // no P & no lateral K move, 
                      board[RookSqr = FromSqr+3 ^StepVec>>1 & 7] -Side-6      // no virgin R in corner  RookSqr,  
                      || board[RookSqr^1] || board[RookSqr^2] )             // no 2 empty sq. next to R  
                    )
                  {
                     Victim += PieceType<5;
                  }                                                     // fake capt. for nonsliding 
                  else SkipSqr=ToSqr ;                                  // enable e.PieceType.               

               } while(!Victim );                                       // if not capt. continue ray 

            }
         }  // (Piece &  Side)

      } while( (FromSqr = (FromSqr +9) & ~bchk)-StartSqr );                // next sqr. of board, wrap  

labelC:
      if (BestScore >INF-bchk || BestScore <bchk-INF) IterDepth=98;           // mate holds to any depth   
      BestScore = BestScore +INF || P==INF ? BestScore  : 0;            // best loses K: (stale)mate 

      if(a->Draft<99) {                                                 // protect game history      
         a->Key=HashKeyHi;
         a->Score=BestScore ;
         a->Draft=IterDepth;                                            // always store in hash tab  
         a->From=BestFrom|8*(BestScore >Alpha)|S*(BestScore <Beta);
         a->To=BestTo;                                                  // move, type (bound/exact), 
      }
      // uncomment for Kibitz  
      // if(prev) sprintf(sbuf, "%2d ply, %9d searched, score=%6d by %c%c%c%c\n",
      //     IterDepth-1, N-S, BestScore , 'a'+(BestFrom & 7),'8'-(BestFrom>>4),'a'+(BestTo & 7),'8'-(BestTo>>4 & 7));

    
       if(prev  && BestFrom!=BestTo) {
         sprintf(sbuf,  "\n%2d ply, searched: %9d ", IterDepth-1, N-S );
         Serial.print(sbuf);
       }
       else if( (N%10000)<1 ) {
         delay(1);
         if(busycount>35) {
            busycount=0;
            Serial.print("\n                            ");
         }
         Serial.print(".");
         busycount++;
      }


   }  // while (iterative deepening loop)

   Side^=24;                                                            // change sides back         
   mfrom=K; mto=L;
   return BestScore += BestScore <Eval;                                 // delayed-loss bonus        
}


//------------------------------------------------------------
void standardBoard() {                                // initial board setup      
   int col=8;                                         // count by column (=file)  
   memset(board, 0, sizeof(board));        
   while(col--) {                                    
      board[col]=(board[col+112]=stdBaseLn[col]+8)+8; // 1st+8th line (=rank): pcs by setup series
      board[col+16]=18;                               // 2nd line: black P-
      board[col+96]=9;                                // 7th line: white P+  
   }  
}


//------------------------------------------------------------
int pieceToVal(char p) {
    // {r,n,b,q,k,+,R,N,B,Q,K,*} => { (+16:) 6,3,5,7,4,2,  (+8:) 6,3,5,7,4,1 }
    // psymbol[]= ".?+nkbrq?*?NKBRQ"
    // 16=white, 8=black
    
    // default=0, for all the rest, e.g.:
    // if(p=='.') return 0;
    // if(p==' ') return 0;
    
    if(p=='r') return 6+16;
    if(p=='n') return 3+16;
    if(p=='b') return 5+16;
    if(p=='q') return 7+16;
    if(p=='k') return 4+16;
    if(p=='+') return 18;  // black P-

    if(p=='R') return 6+8;
    if(p=='N') return 3+8;
    if(p=='B') return 5+8;
    if(p=='Q') return 7+8;
    if(p=='K') return 4+8;
    if(p=='*') return 9;   // white P+ 
    
    //default:
    return 0;

}

//------------------------------------------------------------

    


//------------------------------------------------------------
void centerPointsTable() {                            // center-points table 
   int col=8, x;                                      // (in unused half board[])
   while(col--) {                                                  
     x=8;
      while(x--) { 
        board[16*x + col+8]=(col-4)*(col-4)+(x-3.5)*(x-3.5);           
      }  
   }    
}

//------------------------------------------------------------
void hashTblInit() {
   int h=1035;
   while(h-- > bchk) T[h]=rand()>>9;              // board check: bchk=136=0x88 board system  
}




//------------------------------------------------------------
// chess game user interface
//------------------------------------------------------------



int chess()
{
   int32_t       score, i;
   int16_t       oldto, oldEPSQ;
   char          sbuf[50], sbuf2[50];
   char          cstring[20];
   signed char   oboard[129], spiece;
   float         movecount=1.0;
 
RESTART:
   K=8;
   Side=16;
   memset(board, 0, sizeof(board));
   while(K--)
   {
      board[K]=(board[K+112]=stdBaseLn[K]+8)+8;
      board[K+16]=18;
      board[K+96]=9;                               /* initial board setup*/
      L=8;
      while(L--)board[16*L+K+8]=(K-4)*(K-4)+(L-3.5)*(L-3.5);     /* center-pts table   */
   }                                                             /*(in unused half board[])*/
   N=1035;
   while(N-->bchk)T[N]=rand()>>9;
   
                                                                /* play loop          */
   while(1)                                               
   {
     N=-1;
     
     Serial.print("\n");

     sprintf(sbuf,"     A B C D E F G H \n     --------------- \n"); Serial.print(sbuf);
     while(++N<121) {                                            /* print board */
         if(N&8 && (N+7!=0) ) {sprintf(sbuf,"%3d \n", 1+((120-N)>>4)); Serial.print(sbuf); N+=7; }
         else {
           if(N%8==0) {sprintf(sbuf, "%3d ", 1+((120-N)>>4)); Serial.print(sbuf); }
         sprintf(sbuf," %c", psymbol[board[N]&15]);         Serial.print(sbuf);
         }
     }
     sprintf(sbuf,"     --------------- \n     A B C D E F G H "); Serial.print(sbuf);
     
     if(Side==16) sprintf(sbuf,"\n>  WHITE: ");  else sprintf(sbuf,"\n>   BLACK:  ");
     Serial.print(sbuf);
     
     i = 0;       
     strcpy(cstring,"");
     do   {
       while (Serial.available()==0);       
       cstring[i] = Serial.read();            
       if(cstring[i]==13) {         
         cstring[i]=0;
         break;
       }
       else i++;
     } while(i < 10);

     if(cstring[0]=='Q' && strlen(cstring)==1 ) {
         goto QUIT;
     }
      if(cstring[0]=='R' && strlen(cstring)==1 ) {
         goto RESTART;
     }
     
     K=INF;
     
     if(cstring[0]!=0) {                                   /* parse entered move */
       K= cstring[0]-16*cstring[1]+799;
       L= cstring[2]-16*cstring[3]+799;
     }   
     /*
     Serial.print("\n DEBUG cstring : "); Serial.print(cstring);
     sprintf(sbuf,"\n DEBUG K: %d  \n DEBUG L: %d \n",  K, L);
     Serial.print(sbuf);
     */
     memcpy(oboard, board, sizeof(board));
     oldto=mto;
     oldEPSQ=Rootep;
     
     score=Minimax(-INF, INF, RootEval, Rootep, 1, 3);                              /* think or check & do*/     

     if(score!=15) {                     
        rmvP=S;   
        if(oboard[mto])   rmvP=mto;
        if(mto==oldEPSQ)  rmvP=oldto;
       
        spiece=psymbol[board[mto]&15];
        if(spiece=='*' || spiece=='+') spiece=' ';       
        sprintf(sbuf,"\n\n%4.1f:  %c %c%c", movecount, spiece,'a'+(mfrom&7),'8'-(mfrom>>4) );
        movecount += 0.5;
        if(oboard[mto]) strcat(sbuf," X ");
        else strcat(sbuf,"-");
       
        sprintf(sbuf2,"%c%c ", 'a'+(mto&7),'8'-(mto>>4&7));
        strcat(sbuf, sbuf2);
        Serial.print(sbuf);
     
        sprintf(sbuf, " \nDEBUG: %d to %d  \n", mfrom, mto);
        Serial.print(sbuf);
          sprintf(sbuf,"  EPsq: %c%c (%3d)\n",
                        'a'+(Rootep&7), '8'-(Rootep>>4&7), Rootep); Serial.print(sbuf);
          sprintf(sbuf,"  rmvP: %c%c (%3d)\n\n",
                        'a'+(rmvP&7), '8'-(rmvP>>4&7), rmvP); Serial.print(sbuf);
      }
      else printf("\n\nILLEGAL!\n");     

   }
   return 1;

   QUIT:
   Serial.println("Game quit");
   return 0;
}










/*
void chessPi()
{
   int32_t       eval;
   //int16_t       oldto, oldEPSQ; //debug
   char          sbuf[50], sbuf2[50];
   int16_t       str[20], *ptr;
   float         count=1;
   signed char   oboard[129], spiece;

 
RESTART:
   standardBoard();                                               // standard board setup
   
CUSTOM:      
   eval=INF; 
   centerPointsTable();                                           // center-points table   
                                                                  //(in unused half board[]) 
   hashTblInit();                                                 // init hash table 
   
                                                                        // play loop   
   count=1.0;
   Side=WHITE;                                                                            
   while(1)
   {
     N=-1;

     Serial.print("\n");

     Serial.print("     A B C D E F G H \n     --------------- \n");  
     while(++N<121) {                                            // print board  
         if(N & 8 && (N+7!=0) ) {
             sprintf(sbuf,  "%3d \n", 1+((120-N)>>4)); 
             Serial.print(sbuf);
             N+=7; 
         }
         else {
           if(N%8==0) {
               sprintf(sbuf,  "%3d ", 1+((120-N)>>4)); 
               Serial.print(sbuf);
           }
           sprintf(sbuf, " %c", psymbol[board[N] & 15]);  
           Serial.print(sbuf);      
         }
     }
     Serial.print("     --------------- \n     A B C D E F G H \n");  
     Serial.print(" R_estart  Q_uit  I_nput  S_ide \n"); 
     sprintf(sbuf,  "   move %.1f\n", 0.5+(count/2)); 
     Serial.print(sbuf);

SIDE:
     if(Side==16) Serial.print(">  WHITE: ");  
     else         Serial.print(">  BLACK: ");     
          if(eval==-(INF-1)) {
           Serial.print("--->   CHECKMATE   <---");
        }
        else
        if(eval==0) {
           Serial.print("--->   STALEMATE   <---");
        }
    
     ptr=str;
     while( ( *ptr++ = getchar() ) >10 ) ;
    
     if( str[0]=='Q') return;              //  Q=Quit
     if( str[0]=='R') goto RESTART;        //  R=Restart
     //if( str[0]=='I') { inputNewboard();   //  I=Input TO DO
     //                   goto CUSTOM;    } 
     if( str[0]=='S') { Side^=24;          //  switch side
                        goto SIDE; }      
     if( str[0]==10 ) str[0]=0;            //  Nullstring => auto move generator

     K=INF;
      
     
     if(str[0]!=0) {                                                // parse entered move  
       K= str[0]-16*str[1]+799;
       L= str[2]-16*str[3]+799;     
     }
    
     memcpy(oboard, board, sizeof(board));
     //oldto=mto;
     //oldEPSQ=Rootep;

     eval=Minimax(-INF, INF, RootEval, Rootep, 1, 3);      // think or check & do 
     sprintf(sbuf, "\nmove=%.1f, eval=%d\n", 0.5+(count/2), eval); 
     Serial.print(sbuf);

     if(eval!=15) {
        if(oboard[mto])   rmvP=mto;
        else  rmvP=128;
        //debug: //if(mto==oldEPSQ)  rmvP=oldto;        
        spiece=psymbol[board[mto] & 15];
        if(spiece=='*' || spiece=='+') spiece=' ';           
        sprintf(sbuf,"\n\nmove: %c %c%c", spiece,'a'+(mfrom & 7),'8'-(mfrom>>4));

        if(oboard[mto]) strcat(sbuf," X ");
        else strcat(sbuf,"-");
        sprintf(sbuf2,"%c%c (%d-%d)", 'a'+(mto & 7),'8'-(mto>>4 & 7), mfrom, mto);
        strcat(sbuf, sbuf2);
        Serial.print(sbuf);

        //printf(" \nDEBUG: %d to %d  \n", mfrom, mto);
          Serial.print("  EPsq: ");
          if(Rootep!=128) {
             sprintf(sbuf, "%c%c (%d)", 'a'+(Rootep & 7), '8'-(Rootep>>4 & 7), Rootep);  
             Serial.print(sbuf);
          }
          else   Serial.print ( "(  )");   
       
          Serial.print("  rmvP: ");
          if(rmvP!=128) {
             sprintf(sbuf, "%c%c (%3d)\n\n", 'a'+(rmvP & 7), '8'-(rmvP>>4 & 7), rmvP);  
             Serial.print(sbuf);
          }
          else printf ("(  )"); 
          Serial.print("\n");
          if(eval==-(INF-1)) {
            Serial.print("--->   CHECKMATE   <---");
        }
          else
          if(eval==0) {
            Serial.print("--->   STALEMATE   <---");
          }
          else
            count++;
      }
      
      else sprintf(sbuf,  "\n\nILLEGAL! (eval=%d) \n", eval);
      Serial.print(sbuf);
         
   }
}

*/


//=====================================================================
// setup()
//=====================================================================
void setup() {
   int tftline=10;
   Serial.begin(115200);
   delay(2000);
   Serial.println();
   Serial.println("Serial started!\n");
   int CHESSRUN = 1;

   while (CHESSRUN > 0) {
      CHESSRUN = chess();
      delay(1);
   }
   Serial.println("program stopped");
}

//------------------------------------------------------------
// loop()
//------------------------------------------------------------

void loop() {
   
}
