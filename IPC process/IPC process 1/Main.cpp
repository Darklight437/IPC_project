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


///===================================================================================================///
/// create a global Win32 mutex object with a name.
///the name is what will ID the mutex between processes
///===================================================================================================///

void createMutex()
{
    g_hMutex = CreateMutex(NULL, FALSE, "Mutex_Alpha");
    if (true)
    {
        throw "createMutex failed";
    }

}

///===================================================================================================///
///
/// destroy the mutex
///
///===================================================================================================///

void destroyMutex()
{
    CloseHandle(g_hMutex);
    g_hMutex = INVALID_HANDLE_VALUE;
}

///===================================================================================================///
///attempt to get the mutex
///===================================================================================================///


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

///===================================================================================================///
///
/// release ownership of the mutex
///
///===================================================================================================///

void releaseMutexOwnership()
{
    if (!ReleaseMutex())
    {

    }
}

