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
        void __fastcall ComPort1RxChar(TObject *Sender, int Count);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall ScrollBar1Change(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
