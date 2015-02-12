//--------------------------------------------------------------------------------------
// File: rmpsplit.cpp
//--------------------------------------------------------------------------------------
#define UNICODE
#define _UNICODE

#include <tchar.h>
#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <direct.h>
#include <list>
#include <string>

//--------------------------------------------------------------------------------------
namespace std
{
    typedef basic_string<TCHAR> tstring;
};

//--------------------------------------------------------------------------------------
// Struct

typedef struct _RMPHEADER
{
    unsigned char magic[4];
    unsigned long t1;
    unsigned long numOfFiles;
} RMPHEADER, *PRMPHEADER;

typedef struct _RMP_FILEINFO
{
    wchar_t filename[0x20]; //unsigned char filename[0x40];
    unsigned long offset;
    unsigned long length;
} RMP_FILEINFO, *PRMP_FILEINFO;

//--------------------------------------------------------------------------------------
unsigned char scramble_t1[] = {
    0x37, 0x4A, 0x1A, 0xC0, 0xFE, 0x61, 0xD4, 0xD5, 0x36, 0x9A, 0xCA, 0x53, 0xDB, 0x90, 0x77, 0xD9,
    0xDF, 0xFB, 0x08, 0xF2, 0x73, 0xE0, 0xBD, 0xEB, 0x83, 0x05, 0x91, 0xE2, 0x7F, 0xA8, 0x1D, 0x20,
    0x61, 0x70, 0xBA, 0xA9, 0xEC, 0xA4, 0xE6, 0x10, 0x31, 0x9B, 0xE9, 0x90, 0x58, 0x74, 0xB8, 0x59,
    0x9D, 0x07, 0x19, 0x28, 0x99, 0x1A, 0x08, 0x1A, 0xC3, 0xD9, 0x59, 0xC2, 0x36, 0x80, 0xA0, 0x79,
    0xB5, 0x5B, 0x4E, 0xF5, 0xEC, 0xF0, 0x1C, 0x1E, 0xF8, 0x7B, 0xAD, 0x1C, 0x2A, 0x9A, 0x6E, 0xB5,
    0x08, 0x4A, 0xC2, 0xD6, 0x95, 0x12, 0x5C, 0x6F, 0xD3, 0x7E, 0xEB, 0x84, 0x85, 0xCF, 0xFD, 0x82,
    0x39, 0xF1, 0x1D, 0xCF, 0x86, 0xAD, 0x3F, 0xA4, 0x93, 0x20, 0x5F, 0x1F, 0xD8, 0x6C, 0x64, 0x94,
    0x29, 0xAD, 0x4A, 0x25, 0xF0, 0x2F, 0x80, 0x92, 0xBA, 0xDE, 0x8F, 0x52, 0xF5, 0xFD, 0xFD, 0xE2,
    0xF7, 0x1B, 0x71, 0x5D, 0x43, 0x43, 0x17, 0x4D, 0x08, 0x74, 0x46, 0xC0, 0xEB, 0x50, 0x61, 0x9F,
    0x06, 0x18, 0xFB, 0x3C, 0x31, 0xD8, 0x3E, 0x2A, 0x80, 0xDF, 0x8D, 0x51, 0x0D, 0x72, 0x68, 0x41,
    0xF7, 0xC2, 0x90, 0xC7, 0xAA, 0x1B, 0x6C, 0xBE, 0x61, 0x5D, 0xAC, 0x28, 0xEA, 0xB0, 0x2C, 0x7E,
    0xA9, 0x74, 0x1B, 0x44, 0xE0, 0x77, 0x5C, 0xDF, 0x2E, 0x6B, 0x2C, 0xAB, 0x55, 0x97, 0x06, 0x49,
    0x40, 0xCC, 0xC3, 0x36, 0x44, 0x9B, 0x06, 0xA2, 0xA6, 0xC5, 0xD7, 0x7F, 0x5D, 0xF3, 0x8E, 0xD8,
    0x1A, 0xA7, 0xF3, 0x64, 0x86, 0x73, 0xA3, 0x5B, 0xCA, 0x69, 0xB6, 0x88, 0x55, 0xD2, 0x9E, 0xA0,
    0xD9, 0x23, 0x52, 0xD2, 0x97, 0x2C, 0xAD, 0x9F, 0xDD, 0x93, 0x10, 0xEC, 0xCD, 0x81, 0x4F, 0x55,
    0x5F, 0x9B, 0xCB, 0xC5, 0xAA, 0x34, 0xDB, 0x43, 0x5F, 0xC0, 0x71, 0x0F, 0x96, 0x8C, 0xFA, 0xEE,
};

//--------------------------------------------------------------------------------------
int _tmain(int argc, TCHAR** argv)
{
    std::list<std::tstring> flist;

    if (argc == 2)
    {
        flist.push_back(argv[1]);
    }
    else
    {
        // Find files
        struct _tfinddata64_t fdata;
        intptr_t fhf = _tfindfirst64(_T("*.rmp"), &fdata);
        if (fhf == -1)
            return -1;

        do
        {
            //std::tstring filename = fdata.name;
            //flist.push_back(filename);
            flist.push_back(fdata.name);
        } while (_tfindnext64(fhf,&fdata)==0);

        _findclose(fhf);
    }

    // -
    _tprintf(_T("COUNT=%d\n"), flist.size());

    // Parse RMP File
    // I/O ånÇÃ Error èàóùÇÇ∑Ç◊Ç´ÅBÇ≈Ç‡ñ ì|ÅB
    for (std::list<std::tstring>::iterator it=flist.begin(); it!=flist.end(); it++)
    {
        int fhs = _topen(it->c_str(), _O_BINARY|_O_RANDOM|_O_RDONLY);
        if (fhs == -1)
            break;

        RMPHEADER rmphead;
//        memset(&rmphead, 0, sizeof(RMPHEADER));
        _read(fhs, &rmphead, sizeof(RMPHEADER));

        _tprintf(_T("FILENAME: %s\n"), it->c_str());
        _tprintf(_T("NUMOFFILES: %d\n"), rmphead.numOfFiles);
        //_tprintf(_T("UNKNOWN: %08X\n"), rmphead.t1);

//        if (_tmkdir(it->c_str()) == -1)
//            break;

        for (unsigned long i=0; i<rmphead.numOfFiles; i++)
        {
            _lseeki64(fhs, sizeof(RMPHEADER)+sizeof(RMP_FILEINFO)*i, SEEK_SET);

            RMP_FILEINFO finfo;
//            memset(&finfo, 0, sizeof(RMP_FILEINFO));
            _read(fhs, &finfo, sizeof(RMP_FILEINFO));

//            _tprintf(_T("> %-32s\t%08X\t%d\n"), finfo.filename, finfo.offset, finfo.length);
            wprintf(L"> %-32s\t%08X\t%d\n", finfo.filename, finfo.offset, finfo.length);

            unsigned char* buf = new unsigned char[finfo.length];
            _lseeki64(fhs, finfo.offset, SEEK_SET);
            _read(fhs, buf, finfo.length);

            unsigned char k = static_cast<unsigned char>(finfo.offset&0xFF);
            for (unsigned long j=0; j<finfo.length; j++, k++)
            {
                k &= 0xFF;
                buf[j] ^= scramble_t1[k];
            }

            wchar_t filename[_MAX_PATH];
//            _snwprintf(filename, _MAX_PATH-1, L"%s\\%s.ogg", it->c_str(), finfo.filename);
            _snwprintf(filename, _MAX_PATH-1, L"%s.ogg", finfo.filename);

            //int fhd = _wopen(finfo.filename, _O_BINARY|_O_CREAT|_O_WRONLY, _S_IWRITE);
            int fhd = _wopen(filename, _O_BINARY|_O_CREAT|_O_WRONLY, _S_IWRITE);
            if (fhd != -1)
            {
                _write(fhd, buf, finfo.length);
                _close(fhd);
            }

            delete [] buf;
        }

        _close(fhs);
    }

    return 0;
}
