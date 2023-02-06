#include <Components/SceneComponent.h>
#include <Misc/AutomationTest.h>

#include "UnUtility/Regex.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegex, "UnUtility.TestRegex", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FTestRegex::RunTest(const FString& Parameters)
{
    using namespace sregex;
    SRegex<TCHAR> Regex;
    bool Result;
    Result = Regex.compile(TEXT("a*z+b?"));
    TestTrue(TEXT(""), Result);
    Result = Regex.match(TEXT("azb"));
    TestTrue(TEXT(""), Result);

    Result = Regex.compile(TEXT("a*z"));
    TestTrue(TEXT(""), Result);
    FString Text;
    for(int32 i = 0; i < 65534; ++i) {
        Text += TEXT("a");
    }
    Text += "b";
    Result = Regex.match(*Text);
    TestFalse(TEXT(""), Result);

    Text.RemoveFromEnd(TEXT("b"));
    Text += "z";
    Result = Regex.match(*Text);
    TestTrue(TEXT(""), Result);
    return true;
}
