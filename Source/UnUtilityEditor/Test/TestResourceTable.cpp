#include <Misc/AutomationTest.h>
#include <Components/SceneComponent.h>

#include "UnUtility/RootPointer.h"
#include "UnUtility/ResourceTable.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestResourceTable, "UnUtility.TestResourceTable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FTestResourceTable::RunTest(const FString& Parameters)
{
    static constexpr uint32 NumSamples = 127;
    static constexpr uint32 HalfNumSamples = NumSamples/2;
    struct Item
    {
        Item()
        {
            Component_ = NewObject<USceneComponent>();
        }

        ~Item()
        {
        }

        Item(Item&& Other)
            : Component_(MoveTemp(Other.Component_))
        {
            Other.Component_ = nullptr;
        }

        Item& operator=(Item&& Other)
        {
            Component_ = MoveTemp(Other.Component_);
            return *this;
        }

        void OnPush()
        {
        }

        void OnPop()
        {
        }

        TRootPointer<USceneComponent> Component_;
    };
    FResourceTable<Item> ResourceTable;
    TArray<uint32> Items;
    for (uint32 i = 0; i < NumSamples; ++i) {
        uint32 Handle = ResourceTable.Pop();
        if (!ResourceTable.Valid(Handle)) {
            return false;
        }
        Items.Add(Handle);
    }
    return true;
}

