//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "Unit2.h"
#include "Unit1.h"


using namespace std;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
	GetLogicalDrives(); // ��������� ���������� ������ ��� �������� �����
	Timer1->Enabled=false; // ���������� ������� ��� ������� ���������
	Timer1->Interval=200; // ��������� ��������� ��������
}
//---------------------------------------------------------------------------
void TForm2::GetLogicalDrives()
{
	TCHAR buffer[256]; // ���������� ������ ��� �������� ������ ������
	if (GetLogicalDriveStrings(256, buffer)) // �������, ����������� ������ ������� ������
	{
		ComboBox1->Items->Clear(); //������� ComboBox1

		LPCTSTR currentDrive = buffer; //��������� �� ������� ���� � ������
		while (*currentDrive)   // ���� �� ��������� ����� ������ ������ (����������� ������� ������)
		{
			ComboBox1->Items->Add(currentDrive);  // ��������� ������� ���� � ������ ComboBox1
			currentDrive += _tcslen(currentDrive) + 1;  // ��������� � ���������� ����� � ������
														// (��������� ����� ����� ����� � 1 ��� �������� �������,
														// ����� ��������� ��������� �� ��������� ������� � ������)
		}
	}
}

void __fastcall TForm2::ComboBox1Change(TObject *Sender)
{
	AnsiString selectedDrive = ComboBox1->Text; //����� ��������� ������ � ComboBox1
}
//-----------------------------------------------------------



void __fastcall TForm2::Button2Click(TObject *Sender)
{
	CloseHandle(Disk);
	exit(0);
}
//---------------------------------------------------------------------------


void __fastcall TForm2::Button1Click(TObject *Sender)
{
	Flag = false;
	VirtualStringTree1->Clear(); //������� VSTree
	Timer1->Enabled=true;   //��������� �������
	finder = new Finder(Flag); //������ ������� ������ ��� ������ �����
	fillt = new FillThread(Flag); //������ ������� ������ ��� ������ ������ � VST � DB
}
//---------------------------------------------------------------------------

void __fastcall TForm2::ComboBox1Select(TObject *Sender)
{
	AnsiString Path = L"\\\\.\\" + ComboBox1->Text; //�������������� ����� ����� � Unicode
	Path.Delete(Path.Length(),1);   //�������� ���������� ������� ��� ������� � ������
	LPCWSTR Pathf = WideString(Path).c_bstr(); //�������������� ���������� ��� ����������� ��������� �������� ������ �����
	Disk = CreateFileW(Pathf,GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// ������� �� �������� ����� �� ������
	if (Disk == INVALID_HANDLE_VALUE) //����� ������ � ������ ������������ ������ ���������
	{
		Label2->Caption = "������ �������� ����";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm2::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText)
//������������� ������ �� �������� � VST
{
		stuctureM* structurem = (stuctureM*)Sender->GetNodeData(Node);
	switch (Column) {
		case 0:
			CellText = structurem->value1;
			break;
		case 1:
			CellText = structurem->value2;
			break;
		case 2:
			CellText = structurem->value3;
}
//---------------------------------------------------------------------------
}

void __fastcall TForm2::Timer1Timer(TObject *Sender)
{
	//��������� ��������� �������� ����
	ProgressBar1->Position=int((float(finder->IT->CurrentCluster)/float(finder->IT->ClusterCount))*100);
}
//---------------------------------------------------------------------------


