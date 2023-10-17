//---------------------------------------------------------------------------

#ifndef Unit3H
#define Unit3H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "Classes.h"
using namespace std;
//---------------------------------------------------------------------------
class Finder : public TThread
{
private:
protected:
	void __fastcall Execute();
	int clusternum = 0;
	string hres;
	string res;
public:
	__fastcall Finder(bool CreateSuspended);
	Finder* finder;
	FSMOVER* IT;
};
//---------------------------------------------------------------------------
#endif
