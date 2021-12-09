/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/***************************************************************************/
/* version 3.2 (2000 characters) features:                                 */
/* - recursive negamax search                                              */
/* - quiescence search with recaptures                                     */
/* - recapture extensions                                                  */
/* - (int32ernal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - a hash table storing score and best move                              */
/* - full FIDE rules (expt minor ptomotion) and move-legality checking     */

#define int32 int32_t

#define W(A) while(A)
#define K(A,B) *(int32 *)(T+A+(B&8)+S*(B&7))
#define J(A) K(y+A,b[y])-K(x+A,u)-K(H+A,t)

#define U 16777224
struct _ {
  int32   K,V; 
  char X,Y,D;
} A[U];           /* hash table, 16M+8 entries*/

int32  V=112,
    M=136,
    S=128,
    I=60000,
    C=799,
    Q,N,i;       /* V=0x70=rank mask, M=0x88 */

char O,K,L,
w[]={0,1,1,3,-1,3,5,9},                        /* relative piece values    */
o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
     7,-1,11,6,8,3,6,                          /* 1st dir. in o[] per piece*/
     6,3,5,7,4,5,3,6},                         /* initial piece setup      */
b[129],                                        /* board: half of 16x8+dummy*/
T[1035],                                       /* hash translation table   */

n[]=".?+nkbrq?*?NKBRQ";                        /* piece symbols on printout*/

int32  D(int32  k, int32  q, int32  l, int32  e, int32  J, int32  Z, int32  E, int32  z, int32  n)    /* recursive minimax search, k=moving side, n=depth*/
                        /* (q,l)=window, e=current eval. score, E=e.p. sqr.*/
{                       /* e=score, z=prev.dest; J,Z=hashkeys; return score*/
 int32  j,r,m,v,d,h,i=9,F,G;
 char t,p,u,x,y,X,Y,H,B;
 
 struct _*a=A;
                                               /* lookup pos. in hash table*/
 j=(k*E^J)&U-9;                                /* try 8 consec. locations  */
 W((h=A[++j].K)&&h-Z&&--i);                    /* first empty or match     */
 a+=i?j:0;                                     /* dummy A[0] if miss & full*/
 if(a->K)                                      /* hit: pos. is in hash tab */
 {d=a->D;v=a->V;X=a->X;                        /* examine stored data      */
  if(d>=n)                                     /* if depth sufficient:     */
  {if(v>=l|X&S&&v<=q|X&8)return v;             /* use if window compatible */
   d=n-1;                                      /* or use as iter. start    */
  }X&=~M;Y=a->Y;                               /*      with best-move hint32 */
  Y=d?Y:0;                                     /* don't try best at d=0    */
 }else d=X=Y=0;                                /* start iter., no best yet */
 N++;                                          /* node count (for timing)  */
 W(d++<n|z==8&N<1e7&d<98)                      /* iterative deepening loop */
 {x=B=X;                                       /* start scan at prev. best */
  Y|=8&Y>>4;                                   /* request try noncastl. 1st*/
  m=d>1?-I:e;                                  /* unconsidered:static eval */
  do{u=b[x];                                   /* scan board looking for   */
   if(u&k)                                     /*  own piece (inefficient!)*/
   {r=p=u&7;                                   /* p = piece type (set r>0) */
    j=o[p+16];                                 /* first step vector f.piece*/
    W(r=p>2&r<0?-r:-o[++j])                    /* loop over directions o[] */
    {A:                                        /* resume normal after best */
     y=x;F=G=S;                                /* (x,y)=move, (F,G)=castl.R*/
     do{H=y+=r;                                /* y traverses ray          */
      if(Y&8)H=y=Y&~M;                         /* sneak in prev. best move */
      if(y&M)break;                            /* board edge hit           */
      if(p<3&y==E)H=y^16;                      /* shift capt.sqr. H if e.p.*/
      t=b[H];if(t&k|p<3&!(r&7)!=!t)break;      /* capt. own, bad pawn mode */
      i=99*w[t&7];                             /* value of capt. piece t   */
      if(i<0||E-S&&b[E]&&y-E<2&E-y<2)m=I;      /* K capt. or bad castling  */
      if(m>=l)goto C;                          /* abort on fail high       */
    
      if(h=d-(y!=z))                           /* remaining depth(-recapt.)*/
      {v=p<6?b[x+8]-b[y+8]:0;                  /* center positional pts.   */
       b[G]=b[H]=b[x]=0;b[y]=u&31;             /* do move, strip virgin-bit*/
       if(!(G&M)){b[F]=k+6;v+=30;}             /* castling: put R & score  */
       if(p<3)                                 /* pawns:                   */
       {v-=9*(((x-2)&M||b[x-2]!=u)+            /* structure, undefended    */
              ((x+2)&M||b[x+2]!=u)-1);         /*        squares plus bias */
        if(y+r+1&S){b[y]|=7;i+=C;}             /* promote p to Q, add score*/
       }
       v=-D(24-k,-l-(l>e),m>q?-m:-q,-e-v-i,    /* recursive eval. of reply */
            J+J(0),Z+J(8)+G-S,F,y,h);          /* J,Z: hash keys           */
       v-=v>e;                                 /* delayed-gain penalty     */
       if(z==9)                                /* called as move-legality  */
       {if(v!=-I&x==K&y==L)                    /*   checker: if move found */
        {Q=-e-i;O=F;return l;}                 /*   & not in check, signal */
        v=m;                                   /* (prevent fail-lows on    */
       }                                       /*   K-capt. replies)       */
       b[G]=k+38;b[F]=b[y]=0;b[x]=u;b[H]=t;    /* undo move,G can be dummy */
       if(Y&8){m=v;Y&=~8;goto A;}              /* best=1st done,redo normal*/
       if(v>m){m=v;X=x;Y=y|S&G;}               /* update max, mark with S  */
      }                                        /*          if non castling */
      t+=p<5;                                  /* fake capt. for nonsliding*/
      if(p<3&6*k+(y&V)==S                      /* pawn on 3rd/6th, or      */
          ||(u&~24)==36&j==7&&                 /* virgin K moving sideways,*/
          G&M&&b[G=(x|7)-(r>>1&7)]&32          /* 1st, virgin R in corner G*/
          &&!(b[G^1]|b[G^2])                   /* 2 empty sqrs. next to R  */
      ){F=y;t--;}                              /* unfake capt., enable e.p.*/
     }W(!t);                                   /* if not capt. continue ray*/
  }}}W((x=x+9&~M)-B);                          /* next sqr. of board, wrap */
C:if(m>I/4|m<-I/4)d=99;                        /* mate is indep. of depth  */
  m=m+I?m:-D(24-k,-I,I,0,J,Z,S,S,1)/2;         /* best loses K: (stale)mate*/
  if(!a->K|(a->X&M)!=M|a->D<=d)                /* if new/better type/depth:*/
  {a->K=Z;a->V=m;a->D=d;A->K=0;                /* store in hash,dummy stays*/
   a->X=X|8*(m>q)|S*(m<l);a->Y=Y;              /* empty, type (limit/exact)*/
  }                                            /*    encoded in X S,8 bits */
/*if(z==8)printf("%2d ply, %9d searched, %6d by (%2x,%2x)\n",d-1,N,m,X,Y&0x77);*/
//sprintf(sbuf, "searched: %d\n",N); Serial.print(sbuf);
 }
 if(z&8){K=X; L=Y&~M;}
 return m;                                     
}

void chess()
{
 int32   j, k=8;
 int32   score;
 char sbuf[50], sbuf2[50];
 char oboard[129];
 char oldto, oldEPSQ;
 char cstring[20];

 for(i=0;i<8;i++)
 {b[i]=(b[i+V]=o[i+24]+40)+8;b[i+16]=18;b[i+96]=9;   /* initial board setup*/
  for(j=0;j<8;j++)b[16*j+i+8]=(i-4)*(i-4)+(j-3.5)*(j-3.5);   /* center-pts table   */
 }                                                   /*(in unused half b[])*/
 for(i=M;i<1035;i++)T[i]= rand()>>9;

 W(1)                                                /* play loop          */
 {
   for(i=0;i<121;i++) {
     sprintf(sbuf, " %c",i&8&&(i+=7)?10:n[b[i]&15]); /* print board        */
     Serial.print(sbuf);
   }
  
  
  
                              /* read input line    */  
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
  
  
  N=0;
  if(cstring[0]!=0) {                                   /* parse entered move */
       K= cstring[0]-16*cstring[1]+C;
       L= cstring[2]-16*cstring[3]+C;
      
       Serial.print("\n DEBUG cstring : "); Serial.println(cstring);
       sprintf(sbuf,"\n DEBUG K: %d  \n DEBUG L: %d \n",  K, L);
       Serial.print(sbuf);
  }   
     

  else  /* parse entered move */
  D(k,-I,I,Q,1,1,O,8,0);                            /* or think up one    */
  
  for(i=0;i<U;i++)  A[i].K=0;                               /* clear hash table   */
  if(D(k,-I,I,Q,1,1,O,9,2)==I) k^=24;               /* check legality & do*/
 }
}


  


void setup() {
   Serial.begin(9600);   
}



void loop() {   
   chess();
   
   while(1); 
}

