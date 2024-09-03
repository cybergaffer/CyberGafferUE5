using UnrealBuildTool;

public class CyberGaffer : ModuleRules {
	public CyberGaffer(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Engine",
				"RenderCore",
				"Renderer",
				"RHI",
				"HTTP",
				"Projects",
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
	}
}
