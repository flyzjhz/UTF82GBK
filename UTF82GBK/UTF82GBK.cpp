// UTF82GBK.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "UTF82GBK.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>


#if defined(WIN32) && defined(_WIN32) && !defined(_WIN64) //x86
#ifdef _DEBUG
#if defined(_MT) && defined(_DLL) // MDd
#pragma comment(lib,"x86\\debug-MDd\\libiconv.lib")
#else // MTd
#pragma comment(lib,"x86\\debug-MTd\\libiconv.lib")
#endif // !_MT
#else
#if defined(_MT) && defined(_DLL) // MDd
#pragma comment(lib,"x86\\release-MD\\libiconv.lib")
#else // MTd
#pragma comment(lib,"x86\\release-MT\\libiconv.lib")
#endif // !_MT
#endif // DEBUG
#else //x64
#ifdef _DEBUG
#if defined(_MT) && defined(_DLL) // MDd
#pragma comment(lib,"x64\\debug-MDd\\libiconv.lib")
#else // MTd
#pragma comment(lib,"x64\\debug-MTd\\libiconv.lib")
#endif // !_MT
#else
#if defined(_MT) && defined(_DLL) // MDd
#pragma comment(lib,"x64\\release-MD\\libiconv.lib")
#else // MTd
#pragma comment(lib,"x64\\release-MT\\libiconv.lib")
#endif // !_MT
#endif // DEBUG
#endif

#ifndef ICONV_CONST
# define ICONV_CONST //const
#endif

/*!
对字符串进行语言编码转换
param from		原始编码，比如"GB2312",的按照iconv支持的写
param to		转换的目的编码
param dst		转换后的数据保存到这个指针里，需要在外部分配内存
param dstlen	存储转换后数据的内存大小
param src		原始需要转换的字符串
param srclen    原始字符串长度
*/
int
convert(const char * from, const char * to, char * dst, int dstlen, char * src, int srclen)
{
	iconv_t cd;
	char   *inbuf = src;
	char *outbuf = dst;
	size_t outbufsize = dstlen;
	int status = 0;
	size_t  savesize = 0;
	size_t inbufsize = srclen;
	const char* inptr = inbuf;
	size_t      insize = inbufsize;
	char* outptr = outbuf;
	size_t outsize = outbufsize;

	cd = iconv_open(to, from);
	iconv(cd, NULL, NULL, NULL, NULL);
	if (inbufsize == 0) {
		status = -1;
		goto done;
	}
	while (insize > 0) {
		size_t res = iconv(cd, (ICONV_CONST char**)&inptr, &insize, &outptr, &outsize);
		if (outptr != outbuf) {
			int saved_errno = errno;
			int outsize = outptr - outbuf;
			strncpy(dst + savesize, outbuf, outsize);
			errno = saved_errno;
		}
		if (res == (size_t)(-1)) {
			if (errno == EILSEQ) {
				int one = 1;
				iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &one);
				status = -3;
			}
			else if (errno == EINVAL) {
				if (inbufsize == 0) {
					status = -4;
					goto done;
				}
				else {
					break;
				}
			}
			else if (errno == E2BIG) {
				status = -5;
				goto done;
			}
			else {
				status = -6;
				goto done;
			}
		}
	}
	status = strlen(dst);
done:
	iconv_close(cd);
	return status;

}

//UTF8码转为GB2312码
int U2G(char * dst, int dstlen, char * src, int srclen)
{
	return convert("utf-8", "gb2312", dst, dstlen, src, srclen);
}

//GB2312码转为UTF8码
int G2U(char * dst, int dstlen, char * src, int srclen)
{
	return convert("gb2312", "utf-8", dst, dstlen, src, srclen);
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <io.h>
#include <direct.h>
#include <string>
#include <map>
#if !defined(_UNICODE) && !defined(UNICODE)
#define tstring std::string
#else
#define tstring std::wstring
#endif

typedef std::map<tstring, tstring> KEYVALMAP;
typedef KEYVALMAP::iterator KEYVALMAPIT;
typedef KEYVALMAP::value_type KEYVALMAPPAIR;

//枚举文件夹并获取子文件夹下的文件
void EnumRoot(KEYVALMAP & mFileList, const _TCHAR * ptRootPath = _T("."), const _TCHAR * ptExtension = _T(""))
{
	//用于查找的句柄
	long handle = 0;
	//用于查找的根目录
	_TCHAR tRootName[MAX_PATH] = { 0 };
	//文件信息的结构体
	struct _tfinddata_t tfdt = { 0 };

	//构造查找路径字符串
	_sntprintf(tRootName, MAX_PATH, _T("%s\\*"), ptRootPath);

	//首次查找
	handle = _tfindfirst(tRootName, &tfdt);
	if (handle != (-1))
	{
		//循环查找其他符合的文件，直到找不到其他的为止
		while (!_tfindnext(handle, &tfdt))
		{
			struct _stat st = { 0 };
			_TCHAR tFileName[MAX_PATH] = { 0 };

			if (*tfdt.name != _T('.'))
			{
				_sntprintf(tFileName, MAX_PATH, _T("%s\\%s"), ptRootPath, tfdt.name);
				_tstat(tFileName, &st);

				if ((S_IFDIR & st.st_mode) == S_IFDIR)
				{
					EnumRoot(mFileList, tFileName, ptExtension);
				}
				else
				{
					_TCHAR tDrive[MAX_PATH] = { 0 };
					_TCHAR tPath[MAX_PATH] = { 0 };
					_TCHAR tName[MAX_PATH] = { 0 };
					_TCHAR tExtension[MAX_PATH] = { 0 };
					_tsplitpath(tFileName, tDrive, tPath, tName, tExtension);
					if (!_tcsnicmp(ptExtension, tExtension, _tcslen(tExtension)))
					{
						mFileList.insert(KEYVALMAPPAIR(tFileName, _T("")));
					}
				}
			}
		}
		//关闭查找句柄
		_findclose(handle);
		handle = (-1);
	}
}

void ReadFileType(int & nFileType, _TCHAR * pFileName)
{
	long lCharSize = 0;
	long lFileSize = 0;
	char * pszData = 0;
	FILE * pFile = _tfopen(pFileName, _T("rb"));
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		lFileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		
		pszData = (char *)malloc(lFileSize * sizeof(char));
		if (pszData)
		{
			fread(pszData, lFileSize, sizeof(char), pFile);

			if (!memicmp(pszData, "\xFF\xFE", 2 * sizeof(char)))
			{
				//unicode编码
			} else 
			if (!memicmp(pszData, "\xFE\xFF", 2 * sizeof(char)))
			{
				//unicode-big-endian编码
			} else
			if (!memicmp(pszData, "\xEF\xBB\xBF", 2 * sizeof(char)))
			{
				//utf-8编码
			}
			else
			{

			}

			for (long lIdx = 0; lIdx < lFileSize; lIdx++)
			{
				if ((unsigned char)pszData[lIdx] >= 0x00 &&
					(unsigned char)pszData[lIdx] <= 0x80)
				{
					lCharSize += 1;
				}
			}
			_tprintf(_T("%s\nstrlen:%ld, All:%ld, Characters:%ld, Others:%ld.\n"), pFileName, strlen(pszData), lFileSize, lCharSize, lFileSize - lCharSize);
			free(pszData);
			pszData = 0;
		}
		fclose(pFile);
		pFile = 0;
	}
}

void ConvertMultiFiles(KEYVALMAP & mFileList)
{
	FILE * pSrcFile = 0;
	FILE * pDstFile = 0;
	char * pSrcData = 0;
	char * pDstData = 0;
	long lFileLength = 0;
	long lByteWritten = 0;
	_TCHAR tSrcFileName[MAX_PATH] = { 0 };
	_TCHAR tDstFileName[MAX_PATH] = { 0 };

	int nSrcBufferSize = MAXWORD;
	int nDstBufferSize = nSrcBufferSize * sizeof(nSrcBufferSize);

	pSrcData = (char *)malloc(nSrcBufferSize * sizeof(char));
	pDstData = (char *)malloc(nDstBufferSize * sizeof(char));
	if (pSrcData && pDstData)
	{
		for (KEYVALMAPIT it = mFileList.begin(); it != mFileList.end(); it++)
		{
			_sntprintf(tSrcFileName, MAX_PATH, _T("%s"), it->first.c_str());

			if (it->second.length())
			{
				_sntprintf(tDstFileName, MAX_PATH, _T("%s"), it->second.c_str());
			}
			else
			{
				_sntprintf(tDstFileName, MAX_PATH, _T("%s.1"), tSrcFileName);
				it->second = tDstFileName;
			}
			struct _stat st = { 0 };
			_tstat(tSrcFileName, &st);
			lFileLength = st.st_size;
			pSrcFile = _tfopen(tSrcFileName, _T("r+b"));
			pDstFile = _tfopen(tDstFileName, _T("w+b"));
			if (pSrcFile && pDstFile)
			{
				fseek(pSrcFile, 0, SEEK_END);
				lFileLength = ftell(pSrcFile);

				fseek(pSrcFile, 0, SEEK_SET);
				while (!feof(pSrcFile))
				{
					memset(pSrcData, 0, nSrcBufferSize * sizeof(char));
					memset(pDstData, 0, nDstBufferSize * sizeof(char));

					fread(pSrcData, nSrcBufferSize, sizeof(char), pSrcFile);
					lByteWritten = U2G(pDstData, nDstBufferSize, pSrcData, nSrcBufferSize);
					fwrite(pDstData, lByteWritten, sizeof(char), pDstFile);
				}

				fclose(pDstFile);
				pDstFile = 0;
				fclose(pSrcFile);
				pSrcFile = 0;
			}
		}	
				
		free(pDstData);
		pDstData = 0;
		free(pSrcData);
		pSrcData = 0;
	}

}

int _tmain(int argc, _TCHAR ** argv)
{
	int result = 0;

	int nFileType = 0;
	ReadFileType(nFileType, _T("D:\\ansi.txt"));
	ReadFileType(nFileType, _T("D:\\utf8.txt"));
	ReadFileType(nFileType, _T("D:\\unicode.txt"));
	ReadFileType(nFileType, _T("D:\\unicode-big-endian.txt"));
	ReadFileType(nFileType, _T("D:\\scheduler.py"));

	return result;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	int nFileType = 0;
	ReadFileType(nFileType, _T("D:\\ansi.txt"));
	ReadFileType(nFileType, _T("D:\\utf8.txt"));
	ReadFileType(nFileType, _T("D:\\unicode.txt"));
	ReadFileType(nFileType, _T("D:\\unicode-big-endian.txt"));
	ReadFileType(nFileType, _T("D:\\scheduler.py"));
    // TODO: Place code here.
	if (__argc == 3)
	{
		_TCHAR ** pT = __targv;
		KEYVALMAP mFileList;
		EnumRoot(mFileList, __targv[1], __targv[2]);
		ConvertMultiFiles(mFileList);
	}

	return 0;
}