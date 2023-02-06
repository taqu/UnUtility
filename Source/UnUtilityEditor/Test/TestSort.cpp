#include <Misc/AutomationTest.h>
#include <Components/SceneComponent.h>

#include "UnUtility/RootPointer.h"
#include "UnUtility/Sort.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSort, "UnUtility.TestSort", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FTestSort::RunTest(const FString& Parameters)
{
    return true;
}

