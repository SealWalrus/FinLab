//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "sqlite3.h"
using namespace std;
//---------------------------------------------------------------------------
class FillThread : public TThread
{
private:
	sqlite3_stmt* stmt;
	const char * error;
	int clusternum = 0;
	string hres;
	string res;
protected:
public:
	__fastcall FillThread(bool CreateSuspended);
	void __fastcall UpdateVirtualST();
	void __fastcall Execute();
	void __fastcall FillThread::Serror();
   	void __fastcall FillThread::PBUpdate();
	int rc = 0;
	sqlite3 *db;
};
//---------------------------------------------------------------------------
#endif
