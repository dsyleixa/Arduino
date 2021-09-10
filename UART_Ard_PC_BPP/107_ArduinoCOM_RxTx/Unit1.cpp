//---------------------------------------------------------------------------

// Arduino COM to Borland C++ Builder form 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.

// history:
// 1.0.7  pin-loop, 6x out-pins OUTn(I/O/pwm), &_ALLPINS_=0 when BCB-close
// 1.0.6  output pins DPINn
// 1.0.3  send/receive strings
// 1.0.2  receiving strings, pattern: &varname1=value1;
// 1.0.1  receiving simple Serial char

// ver 1.0.7

//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Unit1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CPort"
#pragma link "CPortCtl"
#pragma resource "*.dfm"
TForm1 *Form1;


#define MSGLEN 1024
#define TOKLEN 30

char msgcstr [MSGLEN]="";

#define iINVALID -29999

int     i0, i1, i2, i3, i4, i5, i6, i7, i8;
int     a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;  // analog pins
double  f0, f1, f2, f3, f4, f5;
unsigned char  b[15];
bool    _PAUSE_ = false;

char    cval[TOKLEN];      // number as cstring


//------------------------------------------------------------

unsigned char bitread(num, pos ) {
   return ( (num >> pos) & 0x01);    
}

//------------------------------------------------------------

int  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}


//------------------------------------------------------------


char * cstringarg( char* haystack, char* vname, char* carg ) {
   int i=0, pos=-1;
   unsigned char  ch=0xff;
   const char*  kini = "&";       // start of varname: '&'
   const char*  kin2 = "?";       // start of varname: '?'
   const char*  kequ = "=";       // end of varname, start of argument: '='
   char  needle[TOKLEN] = "";     // complete pattern:  &varname=abc1234


   strcpy(carg,"");
   strcpy(needle, kini);
   strcat(needle, vname);
   strcat(needle, kequ);
   pos = strstrpos(haystack, needle); 
   if(pos==-1) {
      needle[0]=kin2[0];
      pos = strstrpos(haystack, needle);
      if(pos==-1) return carg;
   }
   pos=pos+strlen(vname)+2; // start of value = kini+vname+kequ   
   while( (ch!='&')&&(ch!='\0') ) {
      ch=haystack[pos+i];    
      if( (ch=='&')||(ch==';')||(ch==' ')||(ch=='\0') ||(ch=='\n')
        ||(i+pos>=strlen(haystack))||(i>TOKLEN-1) ) {
           carg[i]='\0';
           return carg;
      }       
      if( (ch!='&') ) {
          carg[i]=ch;
          i++;       
      }      
   } 
   return carg;
}


//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}


//---------------------------------------------------------------------------


void __fastcall TForm1::ComPort1RxChar(TObject *Sender, int Count)
{
  
  AnsiString rcvStr;

  while( _PAUSE_ );

  ComPort1->ReadStr(rcvStr, 1024);
   // Liest die im Eingangspuffer vorhandenen "Count" -Bytes und kopiert sie
  Label2->Caption=String(rcvStr);

  // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF
  cstringarg(rcvStr.c_str(), Edit3->Text.c_str(), cval); //    0
  if( strlen(cval)>0 ) {
     i0=atoi(cval); Label3->Caption=String(i0);
  }
  cstringarg(rcvStr.c_str(), Edit4->Text.c_str(), cval); //    1
  if( strlen(cval)>0 ) {
     i1=atoi(cval); Label4->Caption=String(i1);
  }
  
  cstringarg(rcvStr.c_str(), Edit5->Text.c_str(), cval); //    2
  if( strlen(cval)>0 ) {
     i2=atoi(cval); Label5->Caption=String(i2);
  }

  cstringarg(rcvStr.c_str(), Edit6->Text.c_str(), cval); //    3
  if( strlen(cval)>0 ) {
     i3=atoi(cval); Label6->Caption=String(i3);
  }

  cstringarg(rcvStr.c_str(), Edit7->Text.c_str(), cval); //    4
  if( strlen(cval)>0 ) {
     i4=atoi(cval); Label7->Caption=String(i4);
  }

  cstringarg(rcvStr.c_str(), Edit8->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     i5=atoi(cval); Label8->Caption=String(i5);
  }

  cstringarg(rcvStr.c_str(), Edit9->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     i6=atoi(cval); Label9->Caption=String(i6);
     char bbuf[100];

     i6=i6 & 0xffff;
     for (int i=0; i<16; i++) {
        b[i] = bitread(i6, i);
     }

     sprintf(bbuf, "%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d",
        b[15],b[14],b[13],b[12],b[11],b[10],b[9],b[8],b[7],b[6],b[5],b[4],b[3],b[2],b[1],b[0]);
     Label11->Caption=String(bbuf);
  }




  // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF
  cstringarg(rcvStr.c_str(), Edit12->Text.c_str(), cval); //    0
  if( strlen(cval)>0 ) {
     f0=atof(cval); Label12->Caption=String(f0);
  }
  cstringarg(rcvStr.c_str(), Edit13->Text.c_str(), cval); //    1
  if( strlen(cval)>0 ) {
     f1=atof(cval); Label13->Caption=String(f1);
  }
  
  cstringarg(rcvStr.c_str(), Edit14->Text.c_str(), cval); //    2
  if( strlen(cval)>0 ) {
     f2=atof(cval); Label14->Caption=String(f2);
  }

  cstringarg(rcvStr.c_str(), Edit15->Text.c_str(), cval); //    3
  if( strlen(cval)>0 ) {
     f3=atof(cval); Label15->Caption=String(f3);
  }

  cstringarg(rcvStr.c_str(), Edit16->Text.c_str(), cval); //    4
  if( strlen(cval)>0 ) {
     f4=atof(cval); Label16->Caption=String(f4);
  }

  cstringarg(rcvStr.c_str(), Edit17->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     f5=atof(cval); Label17->Caption=String(f5);
  }

  cstringarg(rcvStr.c_str(), Edit20->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a0=atof(cval); Label20->Caption=String(a0);
  }

  cstringarg(rcvStr.c_str(), Edit21->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a1=atof(cval); Label21->Caption=String(a1);
  }

  cstringarg(rcvStr.c_str(), Edit22->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a2=atof(cval); Label22->Caption=String(a2);
  }

  cstringarg(rcvStr.c_str(), Edit23->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a3=atof(cval); Label23->Caption=String(a3);
  }

  cstringarg(rcvStr.c_str(), Edit24->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a4=atof(cval); Label24->Caption=String(a4);
  }

  cstringarg(rcvStr.c_str(), Edit25->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     a5=atof(cval); Label25->Caption=String(a5);
  }



}

//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
   ComPort1->ShowSetupDialog();
   ComPort1->Open();

   if(ComPort1->Connected )  {
      Button3->Enabled=true;
      Button4->Enabled=true;

      ScrollBar1->Min=0;
      ScrollBar1->Max=255;
      Button3->Click();

      ScrollBar2->Min=0;
      ScrollBar2->Max=255;
      Button6->Click();

      ScrollBar3->Min=0;
      ScrollBar3->Max=255;
      Button8->Click();

      ScrollBar4->Min=0;
      ScrollBar4->Max=255;
      Button11->Click();

      ScrollBar5->Min=0;
      ScrollBar5->Max=255;
      Button13->Click();

      ScrollBar6->Min=0;
      ScrollBar6->Max=255;
      Button15->Click();

   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender)
{
   Button3->Click();
   ComPort1->Close();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button5Click(TObject *Sender)
{
   if(ComPort1->Connected ) {
      strcat(msgcstr,  "&_ALLPINS_=0;\n" );
      ComPort1->WriteStr(msgcstr);
      Sleep(100);
      ComPort1->Close();
   }

   Application->Terminate();
}
//---------------------------------------------------------------------------





void __fastcall TForm1::Button3Click(TObject *Sender)
{
    if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit1->Text.c_str() );
    
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape1->Brush->Color=clBlack;

      ScrollBar1->Position=0;
      Label1->Caption=String(0);
   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";

      strcpy(cpin, Edit1->Text.c_str() );
 
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );    // debug

      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape1->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar1->Position=255;
      Label1->Caption=String(255);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ScrollBar1Change(TObject *Sender)
{
   ScrollBar1->Min=0;
   ScrollBar1->Max=255;
   int pwm = ScrollBar1->Position;


   char cpin[TOKLEN]="";
   strcpy(cpin, Edit1->Text.c_str() );
  
   
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN];
   sprintf(ibuf, "%d", pwm);
   //itoa( pwm, ibuf, 10);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape1->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label1->Caption=String(pwm);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit18->Text.c_str() );
      
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape2->Brush->Color=clBlack;
      ScrollBar2->Position=0;
      Label18->Caption=String(0);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender)
{
  if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit18->Text.c_str() );
       
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape2->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar2->Position=255;
      Label18->Caption=String(255);
   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::ScrollBar2Change(TObject *Sender)
{
   ScrollBar2->Min=0;
   ScrollBar2->Max=255;
   int pwm = ScrollBar2->Position;

   char cpin[TOKLEN]="";
   strcpy(cpin, Edit18->Text.c_str() );
    
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN]="";
   sprintf(ibuf, "%d", pwm);
   //itoa( pwm, ibuf, 10);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape2->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label18->Caption=String(pwm);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button8Click(TObject *Sender)
{ if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit19->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape3->Brush->Color=clBlack;
      ScrollBar3->Position=0;
      Label19->Caption=String(0);
   }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::Button9Click(TObject *Sender)
{
    if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit19->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape3->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar3->Position=255;
      Label19->Caption=String(255);
   }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ScrollBar3Change(TObject *Sender)
{
   ScrollBar3->Min=0;
   ScrollBar3->Max=255;
   int pwm = ScrollBar3->Position;

   char cpin[TOKLEN]="";
   strcpy(cpin, Edit19->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN]="";
   sprintf(ibuf, "%d", pwm);
   //itoa( pwm, ibuf, 10);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape3->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label19->Caption=String(pwm);
   }
}



//---------------------------------------------------------------------------

void __fastcall TForm1::Edit3Change(TObject *Sender)
{
   Label3->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit4Change(TObject *Sender)
{
    Label4->Caption="";    
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit5Change(TObject *Sender)
{
   Label5->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit6Change(TObject *Sender)
{
   Label6->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit7Change(TObject *Sender)
{
   Label7->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit8Change(TObject *Sender)
{
   Label8->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit9Change(TObject *Sender)
{
   Label9->Caption="";
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Edit12Change(TObject *Sender)
{
   Label12->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit13Change(TObject *Sender)
{
   Label13->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit14Change(TObject *Sender)
{
   Label14->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit15Change(TObject *Sender)
{
   Label15->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit16Change(TObject *Sender)
{
   Label16->Caption="";     
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit17Change(TObject *Sender)
{
   Label17->Caption="";
   Button6->Click();
}




//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1Change(TObject *Sender)
{
   Button6->Click();
   Label1->Caption="";   
   
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit18Change(TObject *Sender)
{
   Button6->Click();
   Label18->Caption="";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit19Change(TObject *Sender)
{
   Button8->Click();
   Label19->Caption="";    
}
//---------------------------------------------------------------------------





void __fastcall TForm1::Edit20Change(TObject *Sender)
{
   Label20->Caption="";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit21Change(TObject *Sender)
{
   Label21->Caption="";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit22Change(TObject *Sender)
{
   Label22->Caption="";   
}

//---------------------------------------------------------------------------

void __fastcall TForm1::Edit23Change(TObject *Sender)
{
   Label23->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit24Change(TObject *Sender)
{
   Label24->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit25Change(TObject *Sender)
{
   Label25->Caption="";
}
//---------------------------------------------------------------------------





void __fastcall TForm1::Button11Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit10->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      Shape4->Brush->Color=clBlack;

      ScrollBar4->Position=0;
      Label27->Caption=String(0);
   }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button12Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";

      strcpy(cpin, Edit10->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape4->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //

      ScrollBar4->Position=255;
      Label27->Caption=String(255);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ScrollBar4Change(TObject *Sender)
{
   ScrollBar4->Min=0;
   ScrollBar4->Max=255;
   int pwm = ScrollBar4->Position;


   char cpin[TOKLEN]="";
   strcpy(cpin, Edit10->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN];
   sprintf(ibuf, "%d", pwm);
   //itoa( pwm, ibuf, 10);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape4->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label27->Caption=String(pwm);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit10Change(TObject *Sender)
{                   
   Button11->Click();
   Label27->Caption="";
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Button13Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit11->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      Shape5->Brush->Color=clBlack;

      ScrollBar5->Position=0;
      Label28->Caption=String(0);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button14Click(TObject *Sender)
{
    if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";

      strcpy(cpin, Edit11->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape5->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //

      ScrollBar5->Position=255;
      Label28->Caption=String(255);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ScrollBar5Change(TObject *Sender)
{
   ScrollBar5->Min=0;
   ScrollBar5->Max=255;
   int pwm = ScrollBar5->Position;


   char cpin[TOKLEN]="";
   strcpy(cpin, Edit11->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN];
   sprintf(ibuf, "%d", pwm);
   //itoa( pwm, ibuf, 10);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape5->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label28->Caption=String(pwm);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit11Change(TObject *Sender)
{                     
   Button13->Click();
   Label28->Caption="";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button15Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";
      strcpy(cpin, Edit26->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      Shape6->Brush->Color=clBlack;

      ScrollBar6->Position=0;
      Label29->Caption=String(0);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button16Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[TOKLEN]="";

      strcpy(cpin, Edit26->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape6->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //

      ScrollBar6->Position=255;
      Label29->Caption=String(255);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ScrollBar6Change(TObject *Sender)
{
   ScrollBar6->Min=0;
   ScrollBar6->Max=255;
   int pwm = ScrollBar6->Position;


   char cpin[TOKLEN]="";
   strcpy(cpin, Edit26->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[TOKLEN];
   sprintf(ibuf, "%d", pwm);
   strcat(msgcstr, ibuf);
   strcat(msgcstr,";\n");

   if(ComPort1->Connected )  {
      ComPort1->WriteStr(msgcstr);
      strcpy(msgcstr, "");

      int p=pwm;
      if(p>1) { if(p<80)p=100; }
      Shape6->Brush->Color= TColor( 65536ul*(1+p/2) + (256ul*p) + (1+p/2) );
      Label29->Caption=String(pwm);
   }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Edit26Change(TObject *Sender)
{
   Button15->Click();
   Label29->Caption="";
}
//---------------------------------------------------------------------------


void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    if(ComPort1->Connected ) {
      strcat(msgcstr,  "&_ALLPINS_=0;\n" );
      ComPort1->WriteStr(msgcstr);
      Sleep(100);
      ComPort1->Close();
   }

   Application->Terminate();
}
//---------------------------------------------------------------------------





