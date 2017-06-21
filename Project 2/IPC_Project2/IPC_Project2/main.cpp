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

bool keyIsDown(int code)
{
    return (GetAsyncKeyState(code) & 0x8000) != 0;
}

void gameLoop_mutex()
{
    std::cout << "======================\n";
    std::cout << "Mutex demo - Process 2\n";
    std::cout << "======================\n\n";
    std::cout << "Usage:\n";
    std::cout << "    - Press '0' to own mutex\n";
    std::cout << "    - Press ESC to quit\n\n";

    std::cout << "Press any key to start game loop...\n\n";
    _getch();

    bool owned = false;
    while (true)
    {
        std::cout << (owned ? "owned" : "not owned") << "\n";

        // Acquire ownership of the mutex as long as the '0' key is held down
        if (!owned && keyIsDown('0'))
        {
            owned = getMutexOwnership();
        }
        if (owned && !keyIsDown('0'))
        {
            releaseMutexOwnership();
            owned = false;
        }

        Sleep(100);

        // Quit on ESCAPE
        if (keyIsDown(VK_ESCAPE))
        {
            break;

        }

    } // while
}

void gameLoop_sharedMemory()
{
    std::cout << "==============================\n";
    std::cout << "Shared Memory demo - Process 2\n";
    std::cout << "==============================\n\n";
    std::cout << "Usage:\n";
    std::cout << "    - Press ESC to quit\n\n";

    std::cout << "Press any key to start game loop...\n\n";
    _getch();

    int frameCount = 0;
    bool owned = false;
    while (true)
    {
        // Attempt to read shared memory and output to cout
        if (getMutexOwnership())
        {
            std::cout << frameCount++ << "  " << (char *)g_pSharedMemory << "\n";
            releaseMutexOwnership();
        }
        else
        {
            std::cout << frameCount++ << "\n";
        }

        Sleep(50);

        // Quit on ESCAPE
        if (keyIsDown(VK_ESCAPE))
        {
            break;

        }

    } // while
}

//==============================================================================///
//                                                                              ///
//==============================================================================///

int main()
{

    try
    {
        // Create a global mutex
        createMutex();

        // Create a global block of shared memory
        createSharedMemory("Foo Memory", 10000);


        gameLoop_mutex(); // Game loop to demo mutex handling
                          //gameLoop_sharedMemory(); // Game loop to demo shared memory

                          // Cleanup
        releaseSharedMemory();
        destroyMutex();

        std::cout << "\nFinished! Press any key...\n\n";
        _getch();
    }
    catch (const char *e)
    {
        std::cout << "EXCEPTION - " << e << "\n\n";
    }

    return 0;
}