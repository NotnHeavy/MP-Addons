//////////////////////////////////////////////////////////////////////////////
// MADE BY NOTNHEAVY. USES GPL-3, AS PER REQUEST OF SOURCEMOD               //
//////////////////////////////////////////////////////////////////////////////

#if defined NOTNHEAVY_MEMORYPOINTER_ADDONS
#endinput
#endif
#define NOTNHEAVY_MEMORYPOINTER_ADDONS

#pragma semicolon true 
#pragma newdecls required

#include <sourcemod>
#include <sdkhooks>
#include <tf2_stocks>

//////////////////////////////////////////////////////////////////////////////
// GLOBALS                                                                  //
//////////////////////////////////////////////////////////////////////////////

enum OSType
{
    OSTYPE_INVALID,

    OSTYPE_WINDOWS,
    OSTYPE_WINDOWS64,
    OSTYPE_LINUX,
    OSTYPE_LINUX64
}
static OSType g_OSType;

static const char GetMemoryPointerAddress_x86[] = "\x8B\x84\x24\x04\x00\x00\x00\xC2\x00\x00";
static const char GetMemoryPointerAddress_x64[] = "\x89\x0A\x48\xC1\xE9\x20\x41\x89\x08\xC2\x00\x00";
static const char FromAddress_x86[] = "\x8B\x84\x24\x04\x00\x00\x00\xC2\x00\x00"; // realistically this is the same as GetMemoryPointerAddress()
static const char FromAddress_x64[] = "\x89\xC8\xC2\x00\x00";

static MemoryPointer MemoryPointer_GetMemoryPointerAddress;
static MemoryPointer MemoryPointer_FromAddress;

static Handle SDKCall_GetMemoryPointerAddress;
static Handle SDKCall_FromAddress;

//////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS                                                       //
//////////////////////////////////////////////////////////////////////////////

static void WriteString(const char[] buffer, int maxlength, MemoryPointer pointer)
{
    for (int i = 0; i < maxlength; ++i)
        pointer.Store(buffer[i], NumberType_Int8, i);
}

//////////////////////////////////////////////////////////////////////////////
// INITIALIZER                                                              //
//////////////////////////////////////////////////////////////////////////////

// Call this in OnPluginStart() to initialize MemoryPointer Addons.
stock void MPAddons_Initialize()
{
    // Load gamedata to get the operating system used.
    GameData gamedata = new GameData("mp_addons");
    if (!gamedata)
        ThrowError("Failed to load gamedata \"%s.txt\"", gamedata);
    g_OSType = view_as<OSType>(gamedata.GetOffset("OSType"));
    delete gamedata;

    // Set up our buffers and SDKCalls.
    switch (MPAddons.GetPointerSize())
    {
        case 4:
        {
            // Initialize and write to the MemoryPointer objects holding our desired functions.
            MemoryPointer_GetMemoryPointerAddress = new MemoryPointer(sizeof(GetMemoryPointerAddress_x86) - 1);
            WriteString(GetMemoryPointerAddress_x86, sizeof(GetMemoryPointerAddress_x86) - 1, MemoryPointer_GetMemoryPointerAddress);

            MemoryPointer_FromAddress = new MemoryPointer(sizeof(FromAddress_x86) - 1);
            WriteString(FromAddress_x86, sizeof(FromAddress_x86) - 1, MemoryPointer_FromAddress);

            // Set up our SDKCalls.
            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddressFromMemoryPointer(MemoryPointer_GetMemoryPointerAddress);
            PrepSDKCall_AddParameter(SDKType_MemoryPointer, SDKPass_Plain);                                             // void* pointer;
            PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);                                             // unsigned
            SDKCall_GetMemoryPointerAddress = EndPrepSDKCall();

            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddressFromMemoryPointer(MemoryPointer_FromAddress);
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);                                              // unsigned address;
            PrepSDKCall_SetReturnInfo(SDKType_MemoryPointer, SDKPass_Plain);                                            // void*
            SDKCall_FromAddress = EndPrepSDKCall();
        }
        case 8:
        {
            // Initialize and write to the MemoryPointer objects holding our desired functions.
            MemoryPointer_GetMemoryPointerAddress = new MemoryPointer(sizeof(GetMemoryPointerAddress_x64));
            WriteString(GetMemoryPointerAddress_x64, sizeof(GetMemoryPointerAddress_x64) - 1, MemoryPointer_GetMemoryPointerAddress);

            MemoryPointer_FromAddress = new MemoryPointer(sizeof(FromAddress_x64) - 1);
            WriteString(FromAddress_x64, sizeof(FromAddress_x64) - 1, MemoryPointer_FromAddress);

            // Set up our SDKCalls.
            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddressFromMemoryPointer(MemoryPointer_GetMemoryPointerAddress);
            PrepSDKCall_AddParameter(SDKType_MemoryPointer, SDKPass_Plain);                                             // void* pointer;
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_ByRef, VDECODE_FLAG_BYREF, VENCODE_FLAG_COPYBACK);   // unsigned& low;
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_ByRef, VDECODE_FLAG_BYREF, VENCODE_FLAG_COPYBACK);   // unsigned& high;
            SDKCall_GetMemoryPointerAddress = EndPrepSDKCall();

            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddressFromMemoryPointer(MemoryPointer_FromAddress);
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);                                              // unsigned low;
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);                                              // unsigned high;
            PrepSDKCall_SetReturnInfo(SDKType_MemoryPointer, SDKPass_Plain);                                            // void*
            SDKCall_FromAddress = EndPrepSDKCall();
        }
    }

}

// Call this in OnPluginEnd() to free MemoryPointer Addons resources.
stock void MPAddons_Free()
{
    delete MemoryPointer_GetMemoryPointerAddress;
}

//////////////////////////////////////////////////////////////////////////////
// STOCKS                                                                   //
//////////////////////////////////////////////////////////////////////////////

methodmap MPAddons
{
    public static OSType GetOperatingSystem()
    {
        return g_OSType;
    }

    public static int GetPointerSize()
    {
        switch (MPAddons.GetOperatingSystem())
        {
            case OSTYPE_WINDOWS, OSTYPE_LINUX:
                return 4;
            case OSTYPE_WINDOWS64, OSTYPE_LINUX64:
                return 8;
            default:
                ThrowError("*WARNING* OPERATING SYSTEM NOT PROPERLY CONFIGURED!");
        }
    }

    public static void GetMemoryPointerAddress(MemoryPointer pointer, any buffer[2])
    {
        switch (MPAddons.GetPointerSize())
        {
            case 4:
            {
                buffer[0] = SDKCall(SDKCall_GetMemoryPointerAddress, pointer);
                buffer[1] = 0;
            }
            case 8:
                SDKCall(SDKCall_GetMemoryPointerAddress, pointer, buffer[0], buffer[1]);
        }
    }

    public static MemoryPointer FromAddress(any buffer[2])
    {
        switch (MPAddons.GetPointerSize())
        {
            case 4:
            {
                MemoryPointer pointer;
                SDKCall(SDKCall_FromAddress, pointer, buffer[0]);
                return pointer;
            }
            case 8:
            {
                MemoryPointer pointer;
                SDKCall(SDKCall_FromAddress, pointer, buffer[0], buffer[1]);
                return pointer;
            }
        }
    }
}