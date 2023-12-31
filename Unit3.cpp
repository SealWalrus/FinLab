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
	FreeOnTerminate=true;  //FreeOnTerminate true, ����� ������������� ���������� ����� ��� ��� ����������.
	std::tuple<string, string, UINT64> MyCortege;  //������������� �������
	auto FS = GetFileSystem(Form2->Disk);  //����������� �������� �������
	IT=new FSMOVER(FS); //�������� ������� FSMOVER ��� ����������� �� �������� �������
	BYTE* cluster; //���������� ���������
	DECOTORA* DEC = new DECOTORA(IT);  //�������� ������� ���������� �� FSMOVER
	UINT64 TCluster = FS->GetClusterCount();  //��������� �������� ���������� ��������� �� �����
	for (DEC->begin(); DEC->check(); DEC->next())  //���������� ������ ��� ������ ��������
	{
		cluster = DEC->GetCluster();  //��������� ����������� ��������, � ������� ���������� ���������
		int clusternum = DEC->GetCurClust();  //��������� ������ ��������
		stringstream result;  //�������� ���������� stringstream ��� �������� �����������
		stringstream hexres;
		for (int j = 0; j < 8; j++)   //���� �������� ������ 8 ���� � �������������� �� � ��������� hex �������
		{
			result << cluster[j];
			hexres <<  hex << setfill('0') << setw(2) << int(cluster[j]) << " ";
		}
		string res = result.str();  //������� ���������� �� ������� stringstream � string
		string hres = hexres.str();
		DEC->signature(res);  //������������� ���������� �������� ��������� � �������� ���
		MyCortege = std::make_tuple(res.c_str(),hres.c_str(),clusternum);  //�������� ������ � ������
		Form2->TQueue.push(MyCortege); //����������� ���������� � �������
	}
	delete IT;  //������� ��������
	delete DEC;
	Form2->Timer1->Enabled=false; //���������� �������, �� ������ �������� ����������� ���������� �������� ����
	Form2->ProgressBar1->Position=int(100); //��������� �������� ���� �� �������� 100 (� ��������� �������� �� ������� �� ����� �� 2-3%)



}
//---------------------------------------------------------------------------
