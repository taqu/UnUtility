using UnrealBuildTool;

public class UnUtilityEditor : ModuleRules
{
	public UnUtilityEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp17;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd", "EditorTests", "UnUtility"});

		PrivateDependencyModuleNames.AddRange(new string[] {});

		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
	}
}
