//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop
#include <tuple>
#include "Unit2.h"
//#include "Unit1.h"
#include "Unit3.h"
//#include "Classes.h"
#include <fstream>
#pragma package(smart_init)
using namespace std;
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Finder::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall Finder::Finder(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall Finder::Execute()
{
	FreeOnTerminate=true;  //FreeOnTerminate true, чтобы автоматически освободить поток при его завершении.
	std::tuple<string, string, UINT64> MyCortege;  //Инициализация кортежа
	auto FS = GetFileSystem(Form2->Disk);  //Определение файловой системы
	IT=new FSMOVER(FS); //Создание объекта FSMOVER для перемещения по файловой системе
	BYTE* cluster; //Присвоение указателя
	DECOTORA* DEC = new DECOTORA(IT);  //Создание объекта декоратора от FSMOVER
	UINT64 TCluster = FS->GetClusterCount();  //Получение значения количества кластеров на диске
	for (DEC->begin(); DEC->check(); DEC->next())  //Реализация метода для поиска сигнатур
	{
		cluster = DEC->GetCluster();  //Получение содержимого кластера, в котором обнаружена сигнатура
		int clusternum = DEC->GetCurClust();  //Получение номера кластера
		stringstream result;  //Создание переменных stringstream для хранения результатов
		stringstream hexres;
		for (int j = 0; j < 8; j++)   //Цикл перебора первых 8 байт и преобразование их в сигнатуру hex формата
		{
			result << cluster[j];
			hexres <<  hex << setfill('0') << setw(2) << int(cluster[j]) << " ";
		}
		string res = result.str();  //Перевод результата из формата stringstream в string
		string hres = hexres.str();
		DEC->signature(res);  //Преобазование результата значения сигнатуры в понятный вид
		MyCortege = std::make_tuple(res.c_str(),hres.c_str(),clusternum);  //Внесение данных в кортеж
		Form2->TQueue.push(MyCortege); //Направление контейнера в очередь
	}
	delete IT;  //Очистка объектов
	delete DEC;
	Form2->Timer1->Enabled=false; //Отключение таймера, по работе которого выполняется наполнение прогресс бара
	Form2->ProgressBar1->Position=int(100); //Доведение прогресс бара до значения 100 (в первичном варианте не доходит до конца на 2-3%)



}
//---------------------------------------------------------------------------
