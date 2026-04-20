// Copyright Yeop. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class InteriorTarget : TargetRules
{
	public InteriorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Interior" });
	}
}
