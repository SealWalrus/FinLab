//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"
#include "Unit3.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall FillThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall FillThread::FillThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	int rc = sqlite3_open("database.db", &db);    // Открываем базу данных "database.db" с использованием SQLite и сохраняем результат в переменную rc, если база отсутствует, то база данных будет создана
	if (rc != SQLITE_OK)    	// Если rc не равно SQLITE_OK, значит произошла ошибка открытия базы данных
	{
		error = "Ошибка открытия базы данных.";
		Synchronize(&Serror);
	}
		else
		{
		error = "Database opened success";
		Synchronize(&Serror);
		}

	//Очистка таблицы
	rc = sqlite3_exec(db,"DELETE FROM my_table;",NULL,NULL,NULL);
	//создание таблицы
	const char* sql = "CREATE TABLE IF NOT EXISTS my_table (col1 TEXT, col2 TEXT, col3 TEXT);";   //Заготовленный запрос
	rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK)
		{
			error = "Ошибка создания таблицы в БД";
			Form2->Label2->Caption=error;
			Synchronize(&Serror);
		}
	else
	{
		error ="Table created succesfully";
		Synchronize(&Serror);
	}
}
//---------------------------------------------------------------------------
void __fastcall FillThread::Execute()
{
	while (!Form2->TQueue.empty() || !Form2->finder->Finished)  //Пока очередь не пуста или поток поиска сигнатур и наполнения очереди не закончил работу
	{
		if (Form2->TQueue.empty())  //Ожидание поступления данных в очередь
		{
			continue;
		}
		auto cortege = Form2->TQueue.frontpop();
		this->res = get<0>(cortege);  //Получение значений из кортежа
		this->hres = get<1>(cortege);
		clusternum = get<2>(cortege);
		const char* sql_insert = "INSERT INTO my_table (col1, col2, col3) VALUES (?, ?, ?);";  //Подготовка запроса SQL на заполнение базы данных
		int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			error = "Cannot prepare statement: ";   //Вывод ошибки в случае некорректного выполнения запроса
			Synchronize(&Serror);
			sqlite3_close(db);
		}
		const char* value1 = res.c_str();
		const char* value2 = hres.c_str();
		int value3 = clusternum;
		Synchronize(&UpdateVirtualST);
		//Заполнение VSTree
		sqlite3_bind_text(stmt, 1, value1, -1, SQLITE_TRANSIENT);  //Преобразование значений для структуры данных
		sqlite3_bind_text(stmt, 2, value2, -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 3, value3);

		rc = sqlite3_step(stmt);  //Изменение состояния БД
		if (rc != SQLITE_DONE) {
			error = "Ошибка вноса данных в БД";
			Synchronize(&Serror);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	}
	sqlite3_close(db);  //Закрытие базы данных
	//Synchronize(&PBUpdate);
	error = "Программа завершила свою работу";
	Synchronize(&Serror);
}
//---------------------------------------------------------------------------

void __fastcall FillThread::UpdateVirtualST()
{
	Form2->VirtualStringTree1->BeginUpdate();  //Начало обновления VST
	PVirtualNode Node = Form2->VirtualStringTree1->AddChild(NULL); //Добавление ноды наследника для наполнения данными
	stuctureM* structurem = (stuctureM*)Form2->VirtualStringTree1->GetNodeData(Node);  // Извлекает данные узла для добавленного узла
	structurem->value1 = UnicodeString(this->res.c_str());   //Заполнение полей значениями
	structurem->value2  = UnicodeString(this->hres.c_str());
	structurem->value3 = this->clusternum;
	Form2->VirtualStringTree1->EndUpdate();  //Окончание процесса обновления
}
//---------------------------------------------------------------------------

void __fastcall FillThread::Serror()
{
	Form2->Label4->Caption = error; //Вывод сообщений о процессе работы программы в текстовое поле в программе
}
//---------------------------------------------------------------------------

void __fastcall FillThread::PBUpdate()
{
	//Form2->ProgressBar1->Position=int(100);
}
//---------------------------------------------------------------------------

