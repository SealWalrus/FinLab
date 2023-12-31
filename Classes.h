// ConsoleApplication1.cpp : ���� ���� �������� ������� "main". ����� ���������� � ������������� ���������� ���������.
//

#include <iostream>
#include <Windows.h>
#include <string>
#include <iomanip>
#include <queue>
#include "mutex"
//#include "sqlite3.h"
using namespace std;

//_______________________________________________________//

class FileSystem {
public:
	virtual string GetName() = 0;
	virtual int GetClusterSize() = 0;
	virtual UINT64 GetClusterCount() = 0;
//	virtual int GetSectorSize() = 0;
	virtual HANDLE GetHandle() = 0;
private:

protected:

};

//�������� ����� �������� ������� EXFAT
class EXFAT:public FileSystem
{
public:
	string GetName() {return Name;} //������� ������ ��� ���������
	int GetClusterSize() {return ClusterSize;}
	UINT64 GetClusterCount() {return ClusterCount;}
	HANDLE GetHandle() {return HandleInfo;}

	EXFAT(HANDLE DiskHandle)
	{
		DWORD off = SetFilePointer(DiskHandle, 0, NULL, FILE_BEGIN); //��������� ������� � ������
		HandleInfo = DiskHandle; //��������� ������ �����
		BYTE* bootrec = new BYTE [512]; //�������� ������ ��� �������� ����������� ������
		DWORD bytesread; //����������� �����
		int readresult=ReadFile(HandleInfo, bootrec,  512, &bytesread, NULL); //����������
		BOOT_EXFAT* mbrex=(BOOT_EXFAT*)bootrec;     // �������� ����������� ������ � ���������������� ���� ��������� BOOT_EXFAT
		ClusterSize = mbrex->sectors_per_cluster* mbrex->bytes_per_sector;   // ��������� ������ ��������, ������� ���������� �������� �� ������� �� ������ ������� � ������
		ClusterCount = mbrex->cluster_count;    // ��������� ���������� ��������� �� �����

		delete[] bootrec;   	// ����������� ������, ���������� ��� ������
	}

	~EXFAT()
	{
		CloseHandle(HandleInfo);
	}

protected:
private:
	string Name = "EXFAT";
	int ClusterSize = 0;
	UINT64 ClusterCount = 0;
	HANDLE HandleInfo=NULL;
	#pragma pack(push,1)
	typedef struct _BOOT_EXFAT
	{
	char jump[3];
	char fsid[8];
	char pad[53];
	UINT64 partoffset;
	UINT64 v_length;
	UINT32 fat_ofset;
	UINT32 fat_length;
	UINT32 cluster_h_offset;
	UINT32 cluster_count; //___________
	UINT32 firstcluster_root;
	UINT32 vol_sn;
	UINT16 fs_rev;
	UINT16 vl_flags;
	UINT8 bytes_per_sector; //___________
	UINT8 sectors_per_cluster; //___________
	} BOOT_EXFAT;
#pragma pack(pop)
};


// �������� ����� �������� ������� NTFS    (����������� ���������� �����������)
class FSNTFS :public FileSystem
{
public:
	string GetName() { return Name; }
	int GetClusterSize() { return ClusterSize; }
	UINT64 GetClusterCount() { return ClusterCount; }
	HANDLE GetHandle() { return HandleInfo; }

	FSNTFS(HANDLE DiskHandle)
	{
		DWORD off = SetFilePointer(DiskHandle, 0, NULL, FILE_BEGIN);
		HandleInfo = DiskHandle;
		BYTE* bootrec = new BYTE[512];
		DWORD bytesread;
		int readresult = ReadFile(HandleInfo, bootrec, 512, &bytesread, NULL);
		BOOT_NTFS* mbr = (BOOT_NTFS*)bootrec;
		ClusterSize = (mbr->sec_size * mbr->secs_cluster);
		ClusterCount = (mbr->num_secs / mbr->secs_cluster);

		delete[] bootrec;
	}

	~FSNTFS()
	{
		CloseHandle(HandleInfo);
	}
private:
	string Name = "NTFS";
	int ClusterSize = 0;
	UINT64 ClusterCount = 0;
	HANDLE HandleInfo = NULL;
#pragma pack(push,1)
	typedef struct _BOOT_NTFS
	{
		BYTE    jump[3];
		BYTE    name[8];
		UINT16  sec_size;
		BYTE    secs_cluster;
		BYTE    reserved_0[7];
		BYTE    media_desc;
		UINT16  reserved_2;
		UINT16  secs_track;
		UINT16  num_heads;
		BYTE    reserved_3[8];
		UINT16  reserved_4;
		UINT16  reserved_5;
		UINT64  num_secs;
		UINT32  mft_clus_lo;
		UINT32  mft_clus_hi;
		UINT32  mft2_clus_lo;
		UINT32  mft2_clus_hi;
		UINT32  mft_rec_size;
		UINT32  buf_size;
		UINT32  volume_id_lo;
		UINT32  volume_id_hi;
	} BOOT_NTFS;
#pragma pack(pop)
protected:
};


// ��������� ����� (���������� ��������� ������� FileSystem � ����������� �� ���� �������� ������� �� �����.)
FileSystem* GetFileSystem(HANDLE DiskHandle)
{
	BYTE NTFS[8] = { 0xEB,0x52,0x90, 0x4E, 0x54, 0x46, 0x53, 0x20 };
	BYTE EXFATsig[8] = { 0xEB,0x76,0x90, 0x45, 0x58, 0x46, 0x41, 0x54 };
	BYTE boot[512]{ 0 };
	int i = 0;
	int ntfs_chk = 0;
	int exfat_chk = 0;
	int flg = 8;
	DWORD BytesRead;
	int readeddata = ReadFile(DiskHandle, boot, 512, &BytesRead, NULL);  // ������ ������ 512 ���� �����
	if (!readeddata || BytesRead != 512)     // ���� ������ ������ ����� ������ �������� ��� ���������� ����������� ���� ����� 512
	{
		cout << "������ ������ ������ �����" << endl;
	}
	for (i = 0; i < 8; i++)   // ��������� ������ �� ����������� ������ � ���������� ����������� �������� ������ NTFS � EXFAT
	{
		if (boot[i] == NTFS[i])ntfs_chk++;
		if (boot[i] == EXFATsig[i])exfat_chk++;
	}
	if (ntfs_chk == 8)
	{
		return new FSNTFS(DiskHandle);    // ���� ��� 8 ���� ������������� ��������� NTFS, ������� � ���������� ��������� ������� FSNTFS
	}
	else
	{
		if (exfat_chk == 8)
		{
			return new EXFAT(DiskHandle);   // ���� ��� 8 ���� ������������� ��������� EXFAT, ������� � ���������� ��������� ������� EXFAT
		}
		else
		{
			cout << "�������� ������� �� ��������������" << endl;   // ���� �� ��������� �� ��������� NTFS, �� EXFAT, ������� ��������� � ���, ��� �������� ������� �� ��������������
		}
	}

}

// ��������   � ���� ���� ���������� ��������� ������ � ��� ���������, ����������� �������� ��� ����������� �� �������� �������
class TERRARIA {
public:
	virtual void begin()=0;    //����. ������� "Virtual": "�� ����������� � ������� ������, �� ������ ������������� � �������� ������ (������� ���������)"
	virtual void next()=0;
	virtual bool check()=0;
	virtual BYTE* current()=0;  //���������� ��������� �� ������ ������
private:
protected:
};

class FSMOVER: public TERRARIA{
public:
	UINT64 ClusterCount = 0;
	int CurrentCluster = 0;
	FSMOVER(FileSystem* FS)
	{
		HNDL = FS->GetHandle();   //��������� ����������� �������� (�����, ���������� ���������, ������ ��������)
		ClusterCount = FS->GetClusterCount();
		ClusterSize = FS->GetClusterSize();
		cluster = new BYTE[ClusterSize]; //��������� ������ ��� 1 ��������
	}
	FSMOVER() = default;
	~FSMOVER()          //����������
	{
		delete[] cluster;
		CloseHandle(HNDL);
	}
	void begin()
	{
		DWORD off = SetFilePointer(HNDL, 0, NULL, FILE_BEGIN);  //����������� ������� � ������ �����
	}
	void next()
	{
		DWORD off = SetFilePointer(HNDL, ClusterSize, NULL, FILE_CURRENT);  //������� ������� � ������ ���������� ��������
		CurrentCluster++;
	}
	bool check()        //�������� ������������ �������� (��������� ��� ���)
	{

		if (CurrentCluster < ClusterCount)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	BYTE* current()  //��������� ������ ������������ ��������
	{
		DWORD ab;
		int ClusterResult = ReadFile(HNDL, cluster, ClusterSize, &ab, NULL);
		DWORD off = SetFilePointer(HNDL,(-1)*ClusterSize, NULL, FILE_CURRENT);
		return cluster;
	}
	int ClstSize()     //��������� ������� ��������
	{
		return ClusterSize;
	}

	int CurClst()    //��������� ������ �������� ��������
	{
		return CurrentCluster;
	}
private:
	int ClusterSize = 0;
	HANDLE HNDL = NULL;
	BYTE* cluster = NULL;
};

//DECORATOR  � ���� ���� ��������� �����-���������, ������� ��������� ���������������� �������� ������ FSMOVER �������� ������ ������ � ����������� ��������� ����� � ��������

class DECOTORA: public FSMOVER{
public:
	DECOTORA(FSMOVER* iterator)
	{
		proved = new BYTE[iterator->ClstSize()];
		this->iterator = iterator;
	}

	~DECOTORA()
	{
		delete[] proved;
	}

	BYTE* GetCluster()
	{
		return proved;
	}

	void begin()
	{
		for (iterator -> begin(); iterator -> check(); iterator->next())   //���������� �������� ��� ������ �������� � ����������, ����� ���������� � ���������� �������� ��� ��������� ���������
		{                                                                  //�������� ���������� �� 1 ������� �����, ��������� ���������� ������ ������� � ����������, ��� ����������� ���������
			proved = iterator->current();
			bool res = prov(proved);
			if (res)
			{
				currentclst = iterator->CurClst();
				break;
			}
		}
	}

	void next()
	{
		iterator->next();
		for (; iterator->check(); iterator->next())
		{
			proved = iterator->current();
			bool res = prov(proved);
			if (res)
			{
				currentclst = iterator->CurClst();
				break;
			}
		}
	}

	int GetCurClust()
	{
		return currentclst;
	}

	bool check()
	{
		return iterator->check();
	}

	BYTE* current()
	{
		return iterator->current();
	}
	void signature(string& sign)  //����� ������, ����������� ������������� ��������� � �������� ����
	{
		bool psdch = true, pngch = true, jpgch = true, bmpch = true;
		for (int i = 0; i < 8; i++)
		{
			if (psdch != false) if (proved[i] != psd[i]) psdch = false;
			if (pngch != false) if (proved[i] != png[i]) pngch = false;
			if (jpgch != false) if (proved[i] != jpg[i]) jpgch = false;
			if (bmpch != false) if (proved[i] != bmp[i]) bmpch = false;
		}
		if (psdch) sign = "PSD";
		if (pngch) sign = "PNG";
		if (jpgch) sign = "JPG";
		if (bmpch) sign = "BMP";
	}
private:
	FSMOVER* iterator = NULL;
	int currentclst = 0;
	BYTE* proved = NULL;
	bool prov(BYTE* arr)  //����� ������, ����������� �������� ���������
	{
		bool psdch = true, pngch = true, jpgch = true, bmpch = true;
		for (int i = 0; i < 8; i++)
		{
			if (psdch != false) if (arr[i] != psd[i]) psdch = false;
			if (pngch != false) if (arr[i] != png[i]) pngch = false;
			if (jpgch != false) if (arr[i] != jpg[i]) jpgch = false;
			if (bmpch != false) if (arr[i] != bmp[i]) bmpch = false;
		}
		if (psdch == true || pngch == true || jpgch == true || bmpch == true) return true;
		else return false;
	}
	BYTE psd[8] = { 0x38, 0x42, 0x50, 0x53, 0x00, 0x01, 0x00, 0x00 }; //������� ���������
	BYTE png[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	BYTE jpg[8] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46 };
	BYTE bmp[8] = { 0x42, 0x4D, 0x3E, 0x7D, 0x00, 0x00, 0x00, 0x00 };

protected:
};

//�������

class tupleque
{
private:
	mutex m;    //���������/��������� ����������� ������� ����
	typedef tuple<string, string, UINT64> MyCortege;  //���������� ���������� �������
	queue<MyCortege> MyCortegeQue;
public:
	MyCortege frontpop()
		{
			lock_guard<mutex> lock(m);  //��������� �� ���� ������� ��������� �������� �����
			tuple <string, string, UINT64> a("","",0);  // ���������� ������� ��������� ������
			if(!MyCortegeQue.empty())
				{
					a=MyCortegeQue.front();
					MyCortegeQue.pop();
				};
			return a;
		}
	void push(MyCortege a)
	{
		lock_guard<mutex> lock(m);
		MyCortegeQue.push(a);
	};
	bool empty()
	{
		if(MyCortegeQue.empty()) return true;
		else return false;
	}
};
