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
	int rc = sqlite3_open("database.db", &db);    // ��������� ���� ������ "database.db" � �������������� SQLite � ��������� ��������� � ���������� rc, ���� ���� �����������, �� ���� ������ ����� �������
	if (rc != SQLITE_OK)    	// ���� rc �� ����� SQLITE_OK, ������ ��������� ������ �������� ���� ������
	{
		error = "������ �������� ���� ������.";
		Synchronize(&Serror);
	}
		else
		{
		error = "Database opened success";
		Synchronize(&Serror);
		}

	//������� �������
	rc = sqlite3_exec(db,"DELETE FROM my_table;",NULL,NULL,NULL);
	//�������� �������
	const char* sql = "CREATE TABLE IF NOT EXISTS my_table (col1 TEXT, col2 TEXT, col3 TEXT);";   //������������� ������
	rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK)
		{
			error = "������ �������� ������� � ��";
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
	while (!Form2->TQueue.empty() || !Form2->finder->Finished)  //���� ������� �� ����� ��� ����� ������ �������� � ���������� ������� �� �������� ������
	{
		if (Form2->TQueue.empty())  //�������� ����������� ������ � �������
		{
			continue;
		}
		auto cortege = Form2->TQueue.frontpop();
		this->res = get<0>(cortege);  //��������� �������� �� �������
		this->hres = get<1>(cortege);
		clusternum = get<2>(cortege);
		const char* sql_insert = "INSERT INTO my_table (col1, col2, col3) VALUES (?, ?, ?);";  //���������� ������� SQL �� ���������� ���� ������
		int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			error = "Cannot prepare statement: ";   //����� ������ � ������ ������������� ���������� �������
			Synchronize(&Serror);
			sqlite3_close(db);
		}
		const char* value1 = res.c_str();
		const char* value2 = hres.c_str();
		int value3 = clusternum;
		Synchronize(&UpdateVirtualST);
		//���������� VSTree
		sqlite3_bind_text(stmt, 1, value1, -1, SQLITE_TRANSIENT);  //�������������� �������� ��� ��������� ������
		sqlite3_bind_text(stmt, 2, value2, -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 3, value3);

		rc = sqlite3_step(stmt);  //��������� ��������� ��
		if (rc != SQLITE_DONE) {
			error = "������ ����� ������ � ��";
			Synchronize(&Serror);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	}
	sqlite3_close(db);  //�������� ���� ������
	//Synchronize(&PBUpdate);
	error = "��������� ��������� ���� ������";
	Synchronize(&Serror);
}
//---------------------------------------------------------------------------

void __fastcall FillThread::UpdateVirtualST()
{
	Form2->VirtualStringTree1->BeginUpdate();  //������ ���������� VST
	PVirtualNode Node = Form2->VirtualStringTree1->AddChild(NULL); //���������� ���� ���������� ��� ���������� �������
	stuctureM* structurem = (stuctureM*)Form2->VirtualStringTree1->GetNodeData(Node);  // ��������� ������ ���� ��� ������������ ����
	structurem->value1 = UnicodeString(this->res.c_str());   //���������� ����� ����������
	structurem->value2  = UnicodeString(this->hres.c_str());
	structurem->value3 = this->clusternum;
	Form2->VirtualStringTree1->EndUpdate();  //��������� �������� ����������
}
//---------------------------------------------------------------------------

void __fastcall FillThread::Serror()
{
	Form2->Label4->Caption = error; //����� ��������� � �������� ������ ��������� � ��������� ���� � ���������
}
//---------------------------------------------------------------------------

void __fastcall FillThread::PBUpdate()
{
	//Form2->ProgressBar1->Position=int(100);
}
//---------------------------------------------------------------------------

