#include <iostream>
#include <string>
#include <conio.h>
#include <Windows.h>

//==============================================================================///
// for the most part this is identical to the other IPC process due to them
// needing exactly the same tools to talk to each other
//therefore i'm cheating and copying a bunch of stuff from jeff's example to
//speed up the process
//==============================================================================///



HANDLE g_hMutex = INVALID_HANDLE_VALUE; 
HANDLE g_hFileMapping = NULL;           
void *g_pSharedMemory = NULL;           


void createMutex()
{

    g_hMutex = CreateMutex(NULL, FALSE, "Mutex_Alpha");
    if (g_hMutex == NULL)
        throw "CreateMutex failed";
}

void destroyMutex()
{
    CloseHandle(g_hMutex);
    g_hMutex = INVALID_HANDLE_VALUE;
}

bool getMutexOwnership()
{
    // We specify a zero timeout becaise in this application, we are polling 
    // for the mutex every frame in our game loop, so a timeout is not necessary.
    // In other circumstances, we may want to wait until ownership of the mutex 
    // has been acquired.
    DWORD timeoutMilliseconds = 0;
    switch (WaitForSingleObject(g_hMutex, timeoutMilliseconds))
    {
    case WAIT_OBJECT_0:
        // Mutex is now owned by this process
        return true;

    case WAIT_TIMEOUT:
        // A timeout means that the mutex is currently owned by some other process
        return false;

    case WAIT_ABANDONED:
        // This means that another process owns the mutx but that process terminated 
        // without releasing the mutex. It may therefore be in an inconsistent state.
        throw "WaitForSingleObject abandoned";

    case WAIT_FAILED:
        throw "WaitForSingleObject failed";

    default: throw "Unknown wait result";
    };

}

void releaseMutexOwnership()
{
    if (!ReleaseMutex(g_hMutex))
        throw "ReleaseMutex failed";
}

void createSharedMemory(const char *name, unsigned numBytes)
{
    // Create a file mapping object
    g_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, numBytes, name);
    if (g_hFileMapping == NULL)
    {
        throw "CreateFileMapping failed";
    }

    // Create a 'view' into the file mapping. The view is effectivel a block of
    // memory backed up by the system paging file.
    g_pSharedMemory = MapViewOfFile(g_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, numBytes);
    if (!g_pSharedMemory)
    {
        throw "MapViewOfFile failed";
    }
}

void releaseSharedMemory()
{
    if (g_pSharedMemory)
    {
        if (UnmapViewOfFile(g_pSharedMemory))
        {
            g_pSharedMemory = nullptr;
        }
        else
        {
            throw "UnmapViewOfFile failed";
        }
    }

    if (g_hFileMapping)
    {
        CloseHandle(g_hFileMapping);
        g_hFileMapping = NULL;
    }
}