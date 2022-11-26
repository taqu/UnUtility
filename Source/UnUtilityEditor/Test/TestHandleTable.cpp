#include <Misc/AutomationTest.h>

#include "UnUtility/HandleTable.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestHandleTable, "UnUtility.TestHandleTable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FTestHandleTable::RunTest(const FString& Parameters)
{
    FHandleTable HandleTable;
    HandleTable.Reserve(111);
    if(128 != HandleTable.Capacity()) {
        return false;
    }
    TArray<uint32> Handles;
    Handles.Reserve(FHandleTable::MaxHandles);
    for(int32 i=0; i<FHandleTable::MaxHandles; ++i) {
        Handles.Add(HandleTable.Pop());
        if(!HandleTable.Valid(Handles[i])) {
            return false;
        }
    }
    for(int32 i = FHandleTable::MaxHandles-1; 0 != i; --i) {
        if(!HandleTable.Valid(Handles[i])) {
            return false;
        }
        uint32 Handle = Handles[i];
        HandleTable.Push(Handle);
        if(HandleTable.Valid(Handles[i])) {
            return false;
        }
    }
    return true;
}
