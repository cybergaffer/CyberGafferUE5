// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)

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
