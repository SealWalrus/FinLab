// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
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

//ДОЧЕРНИЙ КЛАСС ФАЙЛОВОЙ СИСТЕМЫ EXFAT
class EXFAT:public FileSystem
{
public:
	string GetName() {return Name;} //Функции вывода при обращении
	int GetClusterSize() {return ClusterSize;}
	UINT64 GetClusterCount() {return ClusterCount;}
	HANDLE GetHandle() {return HandleInfo;}

	EXFAT(HANDLE DiskHandle)
	{
		DWORD off = SetFilePointer(DiskHandle, 0, NULL, FILE_BEGIN); //Установка каретки в начало
		HandleInfo = DiskHandle; //Получение Хэндла диска
		BYTE* bootrec = new BYTE [512]; //Создание буфера для хранения загрузочной записи
		DWORD bytesread; //Прочитанные байты
		int readresult=ReadFile(HandleInfo, bootrec,  512, &bytesread, NULL); //Заполнение
		BOOT_EXFAT* mbrex=(BOOT_EXFAT*)bootrec;     // Приводим загрузочную запись к соответствующему типу структуры BOOT_EXFAT
		ClusterSize = mbrex->sectors_per_cluster* mbrex->bytes_per_sector;   // Вычисляем размер кластера, умножая количество секторов на кластер на размер сектора в байтах
		ClusterCount = mbrex->cluster_count;    // Сохраняем количество кластеров на диске

		delete[] bootrec;   	// Освобождаем память, выделенную для буфера
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


// ДОЧЕРНИЙ КЛАСС ФАЙЛОВОЙ СИСТЕМЫ NTFS    (Комментарии аналогичны предыдущему)
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


// ФАБРИЧНЫЙ МЕТОД (возвращает экземпляр объекта FileSystem в зависимости от типа файловой системы на диске.)
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
	int readeddata = ReadFile(DiskHandle, boot, 512, &BytesRead, NULL);  // Чтение первых 512 байт диска
	if (!readeddata || BytesRead != 512)     // Если чтение данных диска прошло неудачно или количество прочитанных байт менее 512
	{
		cout << "Ошибка чтения данных диска" << endl;
	}
	for (i = 0; i < 8; i++)   // Сравнение байтов из загрузочной записи с известными сигнатурами файловых систем NTFS и EXFAT
	{
		if (boot[i] == NTFS[i])ntfs_chk++;
		if (boot[i] == EXFATsig[i])exfat_chk++;
	}
	if (ntfs_chk == 8)
	{
		return new FSNTFS(DiskHandle);    // Если все 8 байт соответствуют сигнатуре NTFS, создаем и возвращаем экземпляр объекта FSNTFS
	}
	else
	{
		if (exfat_chk == 8)
		{
			return new EXFAT(DiskHandle);   // Если все 8 байт соответствуют сигнатуре EXFAT, создаем и возвращаем экземпляр объекта EXFAT
		}
		else
		{
			cout << "Файловая система не поддерживается" << endl;   // Если не совпадает ни сигнатура NTFS, ни EXFAT, выводим сообщение о том, что файловая система не поддерживается
		}
	}

}

// ИТЕРАТОР   В этом коде определены интерфейс класса и его наследник, реализующие итератор для перемещения по файловой системе
class TERRARIA {
public:
	virtual void begin()=0;    //Прим. функции "Virtual": "Не реализуется в текущем классе, но обязан реализоваться в потомках класса (задаётся интерфейс)"
	virtual void next()=0;
	virtual bool check()=0;
	virtual BYTE* current()=0;  //Возвращает указатель на массив данных
private:
protected:
};

class FSMOVER: public TERRARIA{
public:
	UINT64 ClusterCount = 0;
	int CurrentCluster = 0;
	FSMOVER(FileSystem* FS)
	{
		HNDL = FS->GetHandle();   //Получение необходимых значений (Хэндл, количество кластеров, размер кластера)
		ClusterCount = FS->GetClusterCount();
		ClusterSize = FS->GetClusterSize();
		cluster = new BYTE[ClusterSize]; //Выделение памяти для 1 кластера
	}
	FSMOVER() = default;
	~FSMOVER()          //Деструктор
	{
		delete[] cluster;
		CloseHandle(HNDL);
	}
	void begin()
	{
		DWORD off = SetFilePointer(HNDL, 0, NULL, FILE_BEGIN);  //Перемещение каретки в начало диска
	}
	void next()
	{
		DWORD off = SetFilePointer(HNDL, ClusterSize, NULL, FILE_CURRENT);  //Перенос каретки в начало следующего кластера
		CurrentCluster++;
	}
	bool check()        //Проверка исследуемого кластера (последний или нет)
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
	BYTE* current()  //Получение данных исследуемого кластера
	{
		DWORD ab;
		int ClusterResult = ReadFile(HNDL, cluster, ClusterSize, &ab, NULL);
		DWORD off = SetFilePointer(HNDL,(-1)*ClusterSize, NULL, FILE_CURRENT);
		return cluster;
	}
	int ClstSize()     //Получение размера кластера
	{
		return ClusterSize;
	}

	int CurClst()    //Получение номера текущего кластера
	{
		return CurrentCluster;
	}
private:
	int ClusterSize = 0;
	HANDLE HNDL = NULL;
	BYTE* cluster = NULL;
};

//DECORATOR  В этом коде определен класс-декоратор, который расширяет функциональность базового класса FSMOVER реализуя задачи поиска и определения сигнатуры файла в кластере

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
		for (iterator -> begin(); iterator -> check(); iterator->next())   //Использует итератор для поиска кластера с сигнатурой, после перемещает к найденному кластеру для обработки сигнатуры
		{                                                                  //Итератор перемещает на 1 кластер диска, декоратор запоминает первый кластер с сигнатурой, для последующей обработки
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
	void signature(string& sign)  //Метод класса, реализующий представление сигнатуры в понятном виде
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
	bool prov(BYTE* arr)  //Метод класса, реализующий проверку сигнатуры
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
	BYTE psd[8] = { 0x38, 0x42, 0x50, 0x53, 0x00, 0x01, 0x00, 0x00 }; //Искомые сигнатуры
	BYTE png[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	BYTE jpg[8] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46 };
	BYTE bmp[8] = { 0x42, 0x4D, 0x3E, 0x7D, 0x00, 0x00, 0x00, 0x00 };

protected:
};

//Очередь

class tupleque
{
private:
	mutex m;    //Открывает/закрывает определённый участок кода
	typedef tuple<string, string, UINT64> MyCortege;  //Объявление контейнера кортежа
	queue<MyCortege> MyCortegeQue;
public:
	MyCortege frontpop()
		{
			lock_guard<mutex> lock(m);  //Действует во всей области видимости закрывая поток
			tuple <string, string, UINT64> a("","",0);  // Объявление шаблона структуры данных
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
