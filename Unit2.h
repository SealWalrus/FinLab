//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "VirtualTrees.hpp"
#include <Vcl.ComCtrls.hpp>
#include "sqlite3.h"
#include <Vcl.ExtCtrls.hpp>
#include "Unit1.h"
#include "Unit3.h"
#include <iostream>
#include "fileapi.h"
#include <sstream>
#include "Windows.h"

typedef struct{  //Структура вносимых данных
	String value1;
	String value2;
	int value3;
}stuctureM;

//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
	TComboBox *ComboBox1;
	TVirtualStringTree *VirtualStringTree1;
	TCheckBox *CheckBox1;
	TCheckBox *CheckBox2;
	TCheckBox *CheckBox3;
	TCheckBox *CheckBox4;
	TProgressBar *ProgressBar1;
	TLabel *Label1;
	TButton *Button1;
	TButton *Button2;
	TLabel *Label2;
	TLabel *Label3;
	TStaticText *StaticText1;
	TStaticText *StaticText2;
	TLabel *Label4;
	TTimer *Timer1;
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ComboBox1Select(TObject *Sender);
	void __fastcall VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText);
	void __fastcall Timer1Timer(TObject *Sender);


private:	// User declarations
	void GetLogicalDrives();

public:		// User declarations
	bool Flag;
	int chng; //Флаг выбора сигнатур
	sqlite3* db;
	sqlite3_stmt* stmt;
	HANDLE Disk;
	FillThread* fillt;
	Finder* finder;
   	tupleque TQueue;
	int rc = 0;
	__fastcall TForm2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
