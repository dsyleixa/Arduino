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
        TGroupBox *GroupBox20;
        TGroupBox *GroupBox21;
        TGroupBox *GroupBox22;
        TGroupBox *GroupBox23;
        TGroupBox *GroupBox24;
        TGroupBox *GroupBox25;
        TGroupBox *GroupBox26;
        TGroupBox *GroupBox27;
        TGroupBox *GroupBox28;
        TEdit *Edit20;
        TEdit *Edit21;
        TEdit *Edit22;
        TEdit *Edit23;
        TEdit *Edit24;
        TEdit *Edit25;
        TLabel *Label20;
        TLabel *Label21;
        TLabel *Label22;
        TLabel *Label23;
        TLabel *Label24;
        TLabel *Label25;
        TLabel *Label10;
        TLabel *Label11;
        TGroupBox *GroupBox10;
        TLabel *Label26;
        TButton *Button10;
        TGroupBox *GroupBox11;
        TScrollBar *ScrollBar4;
        TButton *Button11;
        TButton *Button12;
        TLabel *Label27;
        TShape *Shape4;
        TEdit *Edit27;
        TGroupBox *GroupBox29;
        TLabel *Label28;
        TShape *Shape5;
        TScrollBar *ScrollBar5;
        TButton *Button13;
        TButton *Button14;
        TEdit *Edit28;
        TGroupBox *GroupBox30;
        TLabel *Label29;
        TShape *Shape6;
        TScrollBar *ScrollBar6;
        TButton *Button15;
        TButton *Button16;
        TEdit *Edit29;
        TGroupBox *GroupBox31;
        TEdit *Edit30;
        TButton *Button17;
        TButton *Button18;
        TLabel *Label30;
        TShape *Shape7;
        TScrollBar *ScrollBar7;
        TGroupBox *GroupBox32;
        TGroupBox *GroupBox33;
        TGroupBox *GroupBox34;
        TGroupBox *GroupBox35;
        TButton *Button19;
        TButton *Button20;
        TButton *Button21;
        TButton *Button22;
        TButton *Button23;
        TButton *Button24;
        TButton *Button25;
        TButton *Button26;
        TLabel *Label31;
        TLabel *Label32;
        TLabel *Label33;
        TLabel *Label34;
        TShape *Shape8;
        TShape *Shape9;
        TShape *Shape10;
        TShape *Shape11;
        TEdit *Edit31;
        TEdit *Edit32;
        TEdit *Edit33;
        TEdit *Edit34;
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
        void __fastcall Edit20Change(TObject *Sender);
        void __fastcall Edit21Change(TObject *Sender);
        void __fastcall Edit23Change(TObject *Sender);
        void __fastcall Edit24Change(TObject *Sender);
        void __fastcall Edit25Change(TObject *Sender);
        void __fastcall Edit22Change(TObject *Sender);
        void __fastcall Button11Click(TObject *Sender);
        void __fastcall ScrollBar4Change(TObject *Sender);
        void __fastcall Button12Click(TObject *Sender);
        void __fastcall Edit27Change(TObject *Sender);
        void __fastcall Button13Click(TObject *Sender);
        void __fastcall Button14Click(TObject *Sender);
        void __fastcall ScrollBar5Change(TObject *Sender);
        void __fastcall Edit28Change(TObject *Sender);
        void __fastcall Button15Click(TObject *Sender);
        void __fastcall Button16Click(TObject *Sender);
        void __fastcall ScrollBar6Change(TObject *Sender);
        void __fastcall Edit29Change(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall Button17Click(TObject *Sender);
        void __fastcall Button18Click(TObject *Sender);
        void __fastcall ScrollBar7Change(TObject *Sender);
        void __fastcall Edit30Change(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall ComPort1Exception(TObject *Sender,
          TComExceptions TComException, AnsiString ComportMessage,
          __int64 WinError, AnsiString WinMessage);
        void __fastcall Button19Click(TObject *Sender);
        void __fastcall Button20Click(TObject *Sender);
        void __fastcall Edit31Change(TObject *Sender);
        void __fastcall Button21Click(TObject *Sender);
        void __fastcall Button22Click(TObject *Sender);
        void __fastcall Edit32Change(TObject *Sender);
        void __fastcall Button23Click(TObject *Sender);
        void __fastcall Button24Click(TObject *Sender);
        void __fastcall Edit33Change(TObject *Sender);
        void __fastcall Button25Click(TObject *Sender);
        void __fastcall Button26Click(TObject *Sender);
        void __fastcall Edit34Change(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
