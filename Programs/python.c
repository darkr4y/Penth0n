/* Minimal main program -- everything is loaded from the library */

#include "Python.h"
#include <locale.h>

#ifdef __FreeBSD__
#include <fenv.h>
#endif


// add by me for the path func
#include <Shlwapi.h>
#include <res\PenPyResource.h>

/* add by me */
TCHAR szTempFileName[MAX_PATH] = "Lib.zip";
TCHAR lpTempPathBuffer[MAX_PATH];

int PenPy_Bootstrap() {
	HGLOBAL hResLoad;   // handle to loaded resource
	HRSRC hRes;         // handle/ptr. to res. info. in hExe
	HANDLE hUpdateRes;  // update resource handle
	LPVOID lpResLock;   // pointer to resource data
	DWORD dwResSize = 0;
	
	
	DWORD dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
		lpTempPathBuffer); // buffer for path

	StrCat(lpTempPathBuffer, szTempFileName);

	//printf("%s\n", lpTempPathBuffer);

	// Locate the dialog box resource in the .EXE file.
	hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_PYTHON_CORE), "BINARY");
	if (hRes == NULL)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}

	// Load the dialog box into global memory.
	hResLoad = LoadResource(NULL, hRes);
	if (hResLoad == NULL)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}

	// Lock the dialog box into global memory.
	lpResLock = LockResource(hResLoad);
	if (lpResLock == NULL)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}

	lpResLock = LockResource(hResLoad);
	if (lpResLock == NULL)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}
	dwResSize = SizeofResource(NULL, hRes);
	if (dwResSize <= 0)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}

	HANDLE hFile = CreateFile(lpTempPathBuffer, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Error No: %d \n", GetLastError());
		return 1;
	}
	DWORD dwWrite = 0;
	WriteFile(hFile, lpResLock, dwResSize, &dwWrite, NULL);
	CloseHandle(hFile);

	UnlockResource(lpResLock);
	FreeResource(hRes);
	return 0;
}

void PenPy_Destory() {
	int retval = PathFileExists(lpTempPathBuffer);
	if (retval == 1)
	{
		DeleteFile(lpTempPathBuffer);
	}
}


#ifdef MS_WINDOWS
int
wmain(int argc, wchar_t **argv)
{

	if (PenPy_Bootstrap() == 0) {
		wprintf(L"[+] Penth0n ...\n");
		int sts = Py_Main(argc, argv);
		PenPy_Destory();
		return sts;
	}
    //return Py_Main(argc, argv);
}
#else

int
main(int argc, char **argv)
{
    wchar_t **argv_copy;
    /* We need a second copy, as Python might modify the first one. */
    wchar_t **argv_copy2;
    int i, res;
    char *oldloc;

    /* Force malloc() allocator to bootstrap Python */
    (void)_PyMem_SetupAllocators("malloc");

    argv_copy = (wchar_t **)PyMem_RawMalloc(sizeof(wchar_t*) * (argc+1));
    argv_copy2 = (wchar_t **)PyMem_RawMalloc(sizeof(wchar_t*) * (argc+1));
    if (!argv_copy || !argv_copy2) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    /* 754 requires that FP exceptions run in "no stop" mode by default,
     * and until C vendors implement C99's ways to control FP exceptions,
     * Python requires non-stop mode.  Alas, some platforms enable FP
     * exceptions by default.  Here we disable them.
     */
#ifdef __FreeBSD__
    fedisableexcept(FE_OVERFLOW);
#endif

    oldloc = _PyMem_RawStrdup(setlocale(LC_ALL, NULL));
    if (!oldloc) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    setlocale(LC_ALL, "");
    for (i = 0; i < argc; i++) {
        argv_copy[i] = Py_DecodeLocale(argv[i], NULL);
        if (!argv_copy[i]) {
            PyMem_RawFree(oldloc);
            fprintf(stderr, "Fatal Python error: "
                            "unable to decode the command line argument #%i\n",
                            i + 1);
            return 1;
        }
        argv_copy2[i] = argv_copy[i];
    }
    argv_copy2[argc] = argv_copy[argc] = NULL;

    setlocale(LC_ALL, oldloc);
    PyMem_RawFree(oldloc);

    res = Py_Main(argc, argv_copy);

    /* Force again malloc() allocator to release memory blocks allocated
       before Py_Main() */
    (void)_PyMem_SetupAllocators("malloc");

    for (i = 0; i < argc; i++) {
        PyMem_RawFree(argv_copy2[i]);
    }
    PyMem_RawFree(argv_copy);
    PyMem_RawFree(argv_copy2);
    return res;
}
#endif
