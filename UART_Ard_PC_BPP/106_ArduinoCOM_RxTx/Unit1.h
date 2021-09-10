//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CPort.hpp"
#include "CPortCtl.hpp"
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <ValEdit.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TComPort *ComPort1;
        TGroupBox *GroupBox1;
        TGroupBox *GroupBox2;
        TButton *Button1;
        TButton *Button2;
        TComLed *ComLed1;
        TButton *Button3;
        TButton *Button4;
        TScrollBar *ScrollBar1;
        TLabel *Label1;
        TShape *Shape1;
        TButton *Button5;
        TEdit *Edit1;
        TLabel *Label2;
        TGroupBox *GroupBox3;
        TGroupBox *GroupBox4;
        TGroupBox *GroupBox5;
        TLabel *Label3;
        TEdit *Edit2;
        TEdit *Edit3;
        TEdit *Edit4;
        TLabel *Label4;
        TEdit *Edit5;
        TLabel *Label5;
        TGroupBox *GroupBox6;
        TLabel *Label6;
        TEdit *Edit6;
        TGroupBox *GroupBox7;
        TLabel *Label7;
        TEdit *Edit7;
        TGroupBox *GroupBox8;
        TLabel *Label8;
        TEdit *Edit8;
        TGroupBox *GroupBox9;
        TLabel *Label9;
        TEdit *Edit9;
        TGroupBox *GroupBox10;
        TLabel *Label10;
        TEdit *Edit10;
        TGroupBox *GroupBox11;
        TLabel *Label11;
        TEdit *Edit11;
        TGroupBox *GroupBox12;
        TLabel *Label12;
        TEdit *Edit12;
        TGroupBox *GroupBox13;
        TLabel *Label13;
        TEdit *Edit13;
        TGroupBox *GroupBox14;
        TLabel *Label14;
        TEdit *Edit14;
        TGroupBox *GroupBox15;
        TLabel *Label15;
        TEdit *Edit15;
        TGroupBox *GroupBox16;
        TLabel *Label16;
        TEdit *Edit16;
        TGroupBox *GroupBox17;
        TLabel *Label17;
        TEdit *Edit17;
        TGroupBox *GroupBox18;
        TLabel *Label18;
        TShape *Shape2;
        TButton *Button6;
        TButton *Button7;
        TScrollBar *ScrollBar2;
        TEdit *Edit18;
        TGroupBox *GroupBox19;
        TEdit *Edit19;
        TButton *Button8;
        TButton *Button9;
        TScrollBar *ScrollBar3;
        TLabel *Label19;
        TShape *Shape3;
        void __fastcall ComPort1RxChar(TObject *Sender, int Count);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Edit3Change(TObject *Sender);
        void __fastcall Edit4Change(TObject *Sender);
        void __fastcall Edit5Change(TObject *Sender);
        void __fastcall Edit6Change(TObject *Sender);
        void __fastcall Edit7Change(TObject *Sender);
        void __fastcall Edit8Change(TObject *Sender);
        void __fastcall Edit9Change(TObject *Sender);
        void __fastcall Edit10Change(TObject *Sender);
        void __fastcall Edit11Change(TObject *Sender);
        void __fastcall Edit12Change(TObject *Sender);
        void __fastcall Edit13Change(TObject *Sender);
        void __fastcall Edit14Change(TObject *Sender);
        void __fastcall Edit15Change(TObject *Sender);
        void __fastcall Edit16Change(TObject *Sender);
        void __fastcall Edit17Change(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall ScrollBar1Change(TObject *Sender);
        void __fastcall Button6Click(TObject *Sender);
        void __fastcall Button7Click(TObject *Sender);
        void __fastcall ScrollBar2Change(TObject *Sender);
        void __fastcall Button8Click(TObject *Sender);
        void __fastcall ScrollBar3Change(TObject *Sender);
        void __fastcall Button9Click(TObject *Sender);
        void __fastcall Edit1Change(TObject *Sender);
        void __fastcall Edit18Change(TObject *Sender);
        void __fastcall Edit19Change(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
