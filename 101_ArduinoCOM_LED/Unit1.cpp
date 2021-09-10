//  Arduino remote control by Windows form with widgets
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.

// history:
// 0.0.3  clean-up quit
// 0.0.2  quit button
// 0.0.1  LED buttons only when COM connected
// 0.0.0  basic remote LED on/off 9600 baud

// BorlandCpp-Arduino-RC

// ver 0.0.3

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CPort"
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
   ComPort1->ShowSetupDialog();
   ComPort1->Open();
   if(ComPort1->Connected )  {
      Button3->Enabled=true;
      Button4->Enabled=true;
   }
   Button2->Enabled=true;
   Button5->Enabled=true;
}

//---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender)
{
   Button3->Click();
   ComPort1->Close();

}

//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
   if(ComPort1->Connected )  {
      ComPort1->WriteStr("&LEDBI=0;\n");
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
      ComPort1->WriteStr("&LEDBI=255;\n");
      Shape1->Brush->Color= TColor(65536*(1+127/2)+(256ul*127) + (1+127/2) ); //
      ScrollBar1->Min=0;
      ScrollBar1->Max=255;
      ScrollBar1->Position=255;
      Label1->Caption=String(255);
   }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
   Button3->Click();
   ComPort1->Close();
   Application->Terminate();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ScrollBar1Change(TObject *Sender)
{
   ScrollBar1->Min=0;
   ScrollBar1->Max=255;
   int pwm = ScrollBar1->Position;
   char cmsg[20]="&LEDBI=";
   char buf[20]="";
   itoa( pwm, buf, 10);
   strcat(cmsg, buf);
   strcat(cmsg,";\n");
   if(ComPort1->Connected )  {
      ComPort1->WriteStr(cmsg);
      Shape1->Brush->Color= TColor(65536*(1+pwm/2)+(256ul*pwm) + (1+pwm/2) );

      Label1->Caption=String(pwm);
   }

}

//---------------------------------------------------------------------------


