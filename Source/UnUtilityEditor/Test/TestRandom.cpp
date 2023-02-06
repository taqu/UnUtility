#include <Misc/AutomationTest.h>
#include <Components/SceneComponent.h>

#include "UnUtility/Random.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRandom, "UnUtility.TestRandom", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FTestRandom::RunTest(const FString& Parameters)
{
    FRandomAliasSelect RandomAliasSelect;
    FRandom Random;
    float Weights[4] = {0.0f, 1.0f, 1.0f, 2.0f};
    RandomAliasSelect.Build(sizeof(Weights)/sizeof(Weights[0]), Weights);

    uint32 Counts[4] = {};
    for(uint32 i=0; i<100000; ++i){
        uint32 Index = RandomAliasSelect.Select(Random);
        Counts[Index] += 1;
    }
    for(uint32 i=0; i<4; ++i){
        UE_LOG(LogTemp, Log, TEXT("[%d] %d"), i, Counts[i]);
    }
    return true;
}

