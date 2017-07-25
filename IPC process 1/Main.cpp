/*objectives 

create a mutex object
reserve shared memory 
create shared memory
start polling 



*/




#include <iostream>
#include <sstream>
#include <string>
#include <conio.h>
#include <Windows.h>


//g_ for global variables
HANDLE g_hMutex = INVALID_HANDLE_VALUE; //this is a win32 mutex oject essentially a key that is traded between multiple processes (this applies to multi-threading too
HANDLE g_hFileMapping = NULL;           //a handle to a memory-wrapped file object, i.e. shared memory
void *g_pSharedMemory = NULL;           //A pointer to a block of shared memory backed by a memory wrapped file


//===================================================================================================///
// create a global Win32 mutex object with a name.
// the name is what will ID the mutex between processes
//===================================================================================================///

void createMutex()
{
    g_hMutex = CreateMutex(NULL, FALSE, "MutexAlpha");
    if (g_hMutex == NULL)
    {
        throw "createMutex failed";
    }

}

//===================================================================================================///
// destroy the mutex
//===================================================================================================///

void destroyMutex()
{
    CloseHandle(g_hMutex);
    g_hMutex = INVALID_HANDLE_VALUE;
}

//===================================================================================================///
// attempt to get the mutex
//===================================================================================================///


bool getMutexOwnership()
{
    //there is a timeout option on getting the mutex, this is used for network stuff more than local transfer
    //the program cant continue while it is waiting for the mutex
    //so having a timeout can allow the program to get along with anything else it could be doing without the mutex
    //in this example we will check every frame/loop so the timeout will be instant
    DWORD timeoutMilliseconds = 0;
    switch (WaitForSingleObject(g_hMutex, timeoutMilliseconds))
    {
    case WAIT_OBJECT_0:
        //mutex is owned by this object
        return true;

    case WAIT_TIMEOUT:
        //a timeout means that the mutex is currently held by another process
        return false;

    case WAIT_ABANDONED:
        //this means that another process owns the mutex but that process has terminated
        //without releasing the mutex therefore all manner of oddness has occured
        throw "waitForSingleObject abandoned";

    case WAIT_FAILED:
        throw "WaitForSingleObject failed";



    default:
        throw "Unknown Wait Code";
    };
}

//===================================================================================================///
//
// release ownership of the mutex
//
//===================================================================================================///

void releaseMutexOwnership()
{
    if (!ReleaseMutex(g_hMutex))
    {
        throw "ReleaseMutex failed";
    }
}


//===================================================================================================///
//create a block of shared memory with a specified name and size the name is important
//because it allows the memory to be recognised and shared between processes
//===================================================================================================///

void createSharedMemory(const char *name, unsigned numBytes)
{

    //create a file mapping object
    g_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, numBytes, name);
    if (g_hFileMapping == NULL)
    {
        throw "CreateMappingFile failed";
    }

    //creating a 'view' into the shared mapping
    //a view is effectively a block of memory that has been backed up by the system paging file
    //using the pointer to access the block of data when allowed by the mutex
    g_pSharedMemory = MapViewOfFile(g_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, numBytes);
        if (!g_pSharedMemory)
        {
            throw "MapViewOfFile failed";
        }

}

//===================================================================================================///
// release the memory
//===================================================================================================///

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

//===================================================================================================///
//good old input function for keydown
//===================================================================================================///


bool keyIsDown(int code)
{
    return (GetAsyncKeyState(code) & 0x80000) != 0;
}

//===================================================================================================///
// a game loop that demoes mutex ownership
// copied from jeff for time
//===================================================================================================///



void gameLoop_sharedMemory()
{


    std::cout << "==============================\n";
    std::cout << "Game quiz demo ";
    std::cout << "==============================\n\n";
    std::cout << "Usage:\n";
    std::cout << "    - Press ESC to quit\n\n";

    std::cout << "enter yes for true\n\n";
    std::cin
    _getch();

    int frameCount = 0;
    bool owned = false;
    while (true)
    {
        // Attempt to write to shared memory and output to cout
        if (getMutexOwnership())
        {
            g_pSharedMemory
            //std::cout << frameCount++ << "  " << (char *)g_pSharedMemory << "\n";
            releaseMutexOwnership();
        }


        Sleep(50);

        // Quit on ESCAPE
        if (keyIsDown(VK_ESCAPE))
        {
            break;
        }

    } // while
}

//===================================================================================================///
//                                                                                                   ///
//===================================================================================================///

int main()
{

    try
    {
        // Create a global mutex
        createMutex();

        // Create a global block of shared memory
        createSharedMemory("Foo Memory", 10000);
        *g_pSharedMemory = false;

        //gameLoop_mutex(); // Game loop to demo mutex handling
        gameLoop_sharedMemory(); // Game loop to demo shared memory

                          // Cleanup
        releaseSharedMemory();
        destroyMutex();

        std::cout << "\nFinished! Press any key...\n\n";
        _getch();
    }
    catch (const char *e)
    {
        std::cout << "EXCEPTION - " << e << "\n\n";
        system("pause");
    }

    return 0;
}