/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/***************************************************************************/
/* version 1.6 (1433 non-blank characters) features:                       */
/* - recursive negamax search                                              */
/* - quiescence search with recaptures                                     */
/* - recapture extensions                                                  */
/* - (internal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - full FIDE rules and move-legality checking                            */

/* accepts under-promotions: type 1,2,3 (=R,B,N) after input move          */
/* (input buffer c[] & *P made global, K and N encoding swapped for this)  */


#define W while

int M=136,
    S=128,
    I=8e3,
    C=799,
    Q,
    O,
    K,
    N;   

char L,
     *P,
     w[]={0,1,1,-1,3,3,5,9},                      
     o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0,
          7,-1,6,11,8,3,6,                          
          6,4,5,7,3,5,4,6},                         
     b[129],

     n[]=".?+knbrq?*?KNBRQ",

     c[9];

int D(int k, int q, int l, int e, int E, int z, int n)    
{                       
 int j,r,m,v,d,h,i,F,G,s;
 char t,p,u,x,y,X,Y,H,B;

 q--;                                          
 d=X=Y=0;                                      

 W(d++<n||                                     
   z==8&K==I&&(N<1e6&d<98||                    
   (K=X,L=Y&~M,d=2)))                          
 {x=B=X;                                       
  h=Y&S;                                   
  m=d>1?-I:e;                                  
  N++;                                         
  do{u=b[x];                                   
   if(u&k)                                     
   {r=p=u&7;                                   
    j=o[p+16];                                 
    W(r=p>2&r<0?-r:-o[++j])                    
    {A:                                        
     y=x;F=G=S;                                
     do{                                       
      H=y=h?Y^h:y+r;                        
      if(y&M)break;                            
      m=E-S&&b[E]&&y-E<2&E-y<2?I:m;    /* castling-on-Pawn-check bug fixed */
      if(p<3&y==E)H^=16;                       
      t=b[H];if(t&k|p<3&!(y-x&7)-!t)break;       
      i=99*w[t&7];                             
      m=i<0?I:m;                       /* castling-on-Pawn-check bug fixed */
      if(m>=l)goto C;                          

      if(s=d-(y!=z))                           
      {v=p<6?b[x+8]-b[y+8]:0;
       b[G]=b[H]=b[x]=0;b[y]=u|32;             
       if(!(G&M))b[F]=k+6,v+=30;               
       if(p<3)                                 
       {v-=9*((x-2&M||b[x-2]-u)+               
              (x+2&M||b[x+2]-u)-1);            
        if(y+r+1&S)b[y]|=7,i+=C;               
       }
       v=-D(24-k,-l,m>q?-m:-q,-e-v-i,F,y,s);   
       if(K-I)                                 
       {if(v+I&&x==K&y==L&z==8)                
        {Q=-e-i; O=F;
         if(b[y]-u&7&&P-c>5)b[y]-=c[4]&3;        /* under-promotions */
         return l;
        }v=m;                                   
       }                                       
       b[G]=k+6;b[F]=b[y]=0;b[x]=u;b[H]=t;     
       if(v>m)                         
        m=v,X=x,Y=y|S&F;                       
       if(h){h=0;goto A;}                            
      }
      if(x+r-y|u&32|                           
         p>2&(p-3|j-7||                        
         b[G=x+3^r>>1&7]-k-6                   
         ||b[G^1]|b[G^2])                      
        )t+=p<5;                               
      else F=y;                                
     }W(!t);                                   
  }}}W((x=x+9&~M)-B);                          
C:if(m>I-M|m<M-I)d=98;                         
  m=m+I?m:-D(24-k,-I,I,0,S,S,1);    
 }                                             
 return m+=m<e;                                
}







void chess()
{
   int  k=8;
   int  score, i;
   char sbuf[50], sbuf2[50];
   char oboard[129];
   char oldto, oldEPSQ;
   char cstring[20];

 K=8;
 W(K--)
 {
   b[K]=(b[K+112]=o[K+24]+8)+8;
   b[K+16]=18;b[K+96]=9;
   L=8;
   W(L--) b[16*L+K+8]=(K-4)*(K-4)+(L-3.5)*(L-3.5); 
 }                                                   

 W(1)                                                
 {
   N=-1;
   W(++N<121) {                      // print board
     sprintf(sbuf, " %c",N&8&&(N+=7)?10:n[b[N]&15]);          
     Serial.print(sbuf);
   }   
   
                                     // read input line    
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
  
   
   
   
   K=I;      
   
   if(cstring[0]!=0) {                                   /* parse entered move */
       K= cstring[0]-16*cstring[1]+C;
       L= cstring[2]-16*cstring[3]+C;
   }   
 
   Serial.print("\n DEBUG cstring : "); Serial.println(cstring);
   sprintf(sbuf,"\n DEBUG K: %d  \n DEBUG L: %d \n",  K, L);
   Serial.print(sbuf);
   
   k^= D(k,-I,I,Q,O,8,2)-I ? 0 : 24;
 }
}
 
 
 
 
 
 


void setup() {
   Serial.begin(9600);   
}



void loop() {   
   chess();
   
   while(1); 
}
