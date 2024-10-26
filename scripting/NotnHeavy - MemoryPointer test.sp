// This plugin is both a demonstration of how MemoryPointer works, alongside my "addons".

#include <sourcemod>
#include "mp_addons"

#define PLUGIN_NAME "NotnHeavy - MemoryPointer test"

public void OnPluginStart()
{
    MPAddons_Initialize();
    PrintToServer("--------------------------------------------------------");

    // check platform
    PrintToServer("arch detected: %s\n", ((MPAddons.GetPointerSize() == 8) ? "x86-64" : "x86-32"));
    
    // basic pointer setup
    MemoryPointer pointer = new MemoryPointer(8);                                                   // int* pointer = malloc(sizeof(int) * 2); memset(pointer, 0, sizeof(int) * 2);
                                                                                                    // can also be written as: int* pointer = calloc(2, sizeof(int));
    pointer.Store(0xFFFFFFFF, NumberType_Int32);                                                    // *pointer = 0xFFFFFFFF;
    PrintToServer("*pointer: %i", pointer.Load(NumberType_Int32));                                  // printf("*pointer: %d\n", *pointer);

    // offset by 4 (sizeof(int)) to write to pointer[1]
    pointer.Store(0xFFFFFFFE, NumberType_Int32, 4);                                                 // pointer[1] = 0xFFFFFFFE;
    PrintToServer("pointer[1]: %i", pointer.Load(NumberType_Int32, 4));                             // printf("pointer[1]: %d\n", pointer[1]);

    // you can do the same with MemoryPointer::Offset()
    // however you must delete this handle as well otherwise you will eventually have a memory leak
    // deleting this handle will not delete the memory internally
    MemoryPointer offset_pointer = pointer.Offset(4);
    PrintToServer("<MemoryPointer::Offset()> pointer[1]: %i", pointer.Load(NumberType_Int32, 4));   // printf("<MemoryPointer::Offset()> pointer[1]: %d\n", pointer[1]);
    delete offset_pointer;

    // write another number for the sake of it
    pointer.Store(69420, NumberType_Int32, 4);                                                      // pointer[1] = 69420;
    PrintToServer("pointer[1]: %i", pointer.Load(NumberType_Int32, 4));                             // printf("pointer[1]: %d\n", pointer[1]);

    // delete the pointer handle, which will also free() the memory internally as this handle was instantiated by creating a new memory block
    delete pointer;                                                                                 // free(pointer);
    PrintToServer("");

    // let's create two pointer instances now - one will point to an int, and one will be a pointer to the int pointer
    MemoryPointer p1 = new MemoryPointer(4);                                                        // int* p1 = malloc(sizeof(int));
    MemoryPointer p2 = new MemoryPointer(MPAddons.GetPointerSize());                                // int** p2 = (int*)malloc(sizeof(int*));

    // write to p1
    p1.Store(12345678, NumberType_Int32);                                                           // *p1 = 12345678;

    // write to p2 using MemoryPointer::StoreMemoryPointer()
    p2.StoreMemoryPointer(p1);                                                                      // *p2 = p1;

    // now let's try reading the value of p1 from p2 using MemoryPointer::LoadMemoryPointer()
    // the annoying thing is that this will generate a new handle and therefore you must delete it as well
    MemoryPointer dereferenced = p2.LoadMemoryPointer();
    PrintToServer("**p2: %i", dereferenced.Load(NumberType_Int32));                                 // printf("**p2: %d\n", **p2);
    delete dereferenced;

    // now delete the actual pointers
    delete p2;                                                                                      // free(p2);
    delete p1;                                                                                      // free(p1);
    PrintToServer("");

    // --------------------------------------------------------------------------------------------

    // THIS SECTION IS FOR THOSE INTERESTED IN MY INCLUDE!

    // memorypointer addons: getting address of a pointer
    any buffer[2];
    MemoryPointer newpointer = new MemoryPointer(4);
    MPAddons.GetMemoryPointerAddress(newpointer, buffer);
    PrintToServer("Address of newpointer: 0x%08X%08X", buffer[1], buffer[0]);
    if (MPAddons.GetPointerSize() == 4)
    {
        // let's also try writing to it using StoreToAddress (although this will throw a warning as it is deprecated)
        StoreToAddress(buffer[0], 0xDEADBEEF, NumberType_Int32);
        PrintToServer("*newpointer (as hex): 0x%X", newpointer.Load(NumberType_Int32));
    }
    delete newpointer;
    PrintToServer("");

    // memorypointer addons: generating a memorypointer from an address
    any buffer2[2] = { 0xC0FFEE, 0 };
    any buffer3[2];
    MemoryPointer sdkcallpointer = MPAddons.FromAddress(buffer2);
    MPAddons.GetMemoryPointerAddress(sdkcallpointer, buffer3);
    PrintToServer("Address of sdkcallpointer: 0x%08X%08X", buffer3[1], buffer3[0]);
    delete sdkcallpointer;
    PrintToServer("");

    PrintToServer("\"%s\" has loaded.\n--------------------------------------------------------", PLUGIN_NAME);
}

public void OnMapStart()
{
    // basic entity code
    MemoryPointer pointer = new MemoryPointer(MPAddons.GetPointerSize()); // this is 4 on 32-bit (where i'm testing) but 8 on 64-bit
    pointer.StoreEntityToHandle(0); // world - if we use an entity index that doesn't exist, this will error!
    PrintToServer("entity in pointer: %i", pointer.LoadEntityFromHandle());
    if (IsValidEntity(1))
    {
        pointer.StoreEntityToHandle(1);
        PrintToServer("new entity in pointer: %i", pointer.LoadEntityFromHandle());   
    }
}

public void OnPluginEnd()
{
    MPAddons_Free();
}