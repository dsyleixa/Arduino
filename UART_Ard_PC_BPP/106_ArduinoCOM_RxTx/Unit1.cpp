//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CPort"
#pragma link "CPortCtl"
#pragma resource "*.dfm"
TForm1 *Form1;


#define MSGLEN 1024
char msgcstr [MSGLEN]="";

#define iINVALID -29999

int     i0, i1, i2, i3, i4, i5, i6, i7, i8;
double  d0, d1, d2, d3, d4, d5;
char    cval[20];      // number as cstring


//------------------------------------------------------------

int  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}


//------------------------------------------------------------

#define TOKLEN 30

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
  }

  cstringarg(rcvStr.c_str(), Edit10->Text.c_str(), cval); //    7
  if( strlen(cval)>0 ) {
     i7=atoi(cval); Label10->Caption=String(i7);
  }

  cstringarg(rcvStr.c_str(), Edit11->Text.c_str(), cval); //    8
  if( strlen(cval)>0 ) {
     i8=atoi(cval); Label11->Caption=String(i8);
  }

  // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF
  cstringarg(rcvStr.c_str(), Edit12->Text.c_str(), cval); //    0
  if( strlen(cval)>0 ) {
     d0=atof(cval); Label12->Caption=String(d0);
  }
  cstringarg(rcvStr.c_str(), Edit13->Text.c_str(), cval); //    1
  if( strlen(cval)>0 ) {
     d1=atof(cval); Label13->Caption=String(d1);
  }
  
  cstringarg(rcvStr.c_str(), Edit14->Text.c_str(), cval); //    2
  if( strlen(cval)>0 ) {
     d2=atof(cval); Label14->Caption=String(d2);
  }

  cstringarg(rcvStr.c_str(), Edit15->Text.c_str(), cval); //    3
  if( strlen(cval)>0 ) {
     d3=atof(cval); Label15->Caption=String(d3);
  }

  cstringarg(rcvStr.c_str(), Edit16->Text.c_str(), cval); //    4
  if( strlen(cval)>0 ) {
     d4=atof(cval); Label16->Caption=String(d4);
  }

  cstringarg(rcvStr.c_str(), Edit17->Text.c_str(), cval); //    5
  if( strlen(cval)>0 ) {
     d5=atof(cval); Label17->Caption=String(d5);
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
      Button2->Enabled=true;
      Button5->Enabled=true;

      Button3->Click();
      Button6->Click();
      Button8->Click();

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
   Button3->Click();
   ComPort1->Close();
   Application->Terminate();
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

void __fastcall TForm1::Edit10Change(TObject *Sender)
{
   Label10->Caption="";        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit11Change(TObject *Sender)
{
   Label11->Caption="";        
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
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button3Click(TObject *Sender)
{
    if(ComPort1->Connected )  {
      char cpin[30]="";
      strcpy(cpin, Edit1->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape1->Brush->Color=clBlack;
      ScrollBar1->Min=0;
      ScrollBar1->Max=255;
      ScrollBar1->Position=0;
      Label1->Caption=String(0);
   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      char cpin[30]="";
      strcpy(cpin, Edit1->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape1->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar1->Min=0;
      ScrollBar1->Max=255;
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

   char cpin[30]="";
   strcpy(cpin, Edit1->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[20]="";
   itoa( pwm, ibuf, 10);
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
      char cpin[30]="";
      strcpy(cpin, Edit18->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape2->Brush->Color=clBlack;
      ScrollBar2->Min=0;
      ScrollBar2->Max=255;
      ScrollBar2->Position=0;
      Label18->Caption=String(0);
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender)
{
  if(ComPort1->Connected )  {
      char cpin[30]="";
      strcpy(cpin, Edit18->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape2->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar2->Min=0;
      ScrollBar2->Max=255;
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

   char cpin[30]="";
   strcpy(cpin, Edit18->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[20]="";
   itoa( pwm, ibuf, 10);
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
      char cpin[30]="";
      strcpy(cpin, Edit19->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );
      strcat(msgcstr,  "=0;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");
      Shape3->Brush->Color=clBlack;
      ScrollBar3->Min=0;
      ScrollBar3->Max=255;
      ScrollBar3->Position=0;
      Label19->Caption=String(0);
   }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::Button9Click(TObject *Sender)
{
    if(ComPort1->Connected )  {
      char cpin[30]="";
      strcpy(cpin, Edit19->Text.c_str() );
      strcat(msgcstr,  "&" );
      strcat(msgcstr,  cpin );

      strcat(msgcstr,  "=255;\n" );

      //strcat(msgcstr,  "&LEDBI=255;\n" );
      ComPort1->WriteStr(msgcstr);

      strcpy(msgcstr, "");

      Shape3->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar3->Min=0;
      ScrollBar3->Max=255;
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

   char cpin[30]="";
   strcpy(cpin, Edit19->Text.c_str() );
   strcat(msgcstr,  "&" );
   strcat(msgcstr,  cpin );
   strcat(msgcstr,  "=" );

   char ibuf[20]="";
   itoa( pwm, ibuf, 10);
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

void __fastcall TForm1::Edit1Change(TObject *Sender)
{
   Label1->Caption="";
   Button6->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit18Change(TObject *Sender)
{
   Label18->Caption="";
   Button6->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit19Change(TObject *Sender)
{
   Label19->Caption="";
   Button8->Click();
}
//---------------------------------------------------------------------------

