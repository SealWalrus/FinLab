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
	GetLogicalDrives(); // Получение логических дисков при создании формы
	Timer1->Enabled=false; // Выключение таймера при запуске программы
	Timer1->Interval=200; // Установка интервала сработки
}
//---------------------------------------------------------------------------
void TForm2::GetLogicalDrives()
{
	TCHAR buffer[256]; // Объявление буфера для хранения списка дисков
	if (GetLogicalDriveStrings(256, buffer)) // Функция, заполняющая список именами дисков
	{
		ComboBox1->Items->Clear(); //Очистка ComboBox1

		LPCTSTR currentDrive = buffer; //Указатель на текущий диск в буфере
		while (*currentDrive)   // Пока не достигнут конец списка дисков (завершающий нулевой символ)
		{
			ComboBox1->Items->Add(currentDrive);  // Добавляем текущий диск в список ComboBox1
			currentDrive += _tcslen(currentDrive) + 1;  // Переходим к следующему диску в буфере
														// (добавляем длину имени диска и 1 для нулевого символа,
														// чтобы указатель сместился на следующий элемент в буфере)
		}
	}
}

void __fastcall TForm2::ComboBox1Change(TObject *Sender)
{
	AnsiString selectedDrive = ComboBox1->Text; //Вывод доступных дисков в ComboBox1
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
	VirtualStringTree1->Clear(); //Очистка VSTree
	Timer1->Enabled=true;   //Включение таймера
	finder = new Finder(Flag); //Запуск первого потока для обхода диска
	fillt = new FillThread(Flag); //Запуск второго потока для вывода данных в VST и DB
}
//---------------------------------------------------------------------------

void __fastcall TForm2::ComboBox1Select(TObject *Sender)
{
	AnsiString Path = L"\\\\.\\" + ComboBox1->Text; //Преобразование имени диска в Unicode
	Path.Delete(Path.Length(),1);   //Удаление последнего символа для доступа к файлам
	LPCWSTR Pathf = WideString(Path).c_bstr(); //Преобразование директории для корректного понимания функцией чтения диска
	Disk = CreateFileW(Pathf,GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// Функция на открытие диска на чтение
	if (Disk == INVALID_HANDLE_VALUE) //Вывод ошибки в случае некорректной работы программы
	{
		Label2->Caption = "Ошибка открытия тома";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm2::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText)
//Распределение текста по колонкам в VST
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
	//Изменение состояния прогресс бара
	ProgressBar1->Position=int((float(finder->IT->CurrentCluster)/float(finder->IT->ClusterCount))*100);
}
//---------------------------------------------------------------------------


