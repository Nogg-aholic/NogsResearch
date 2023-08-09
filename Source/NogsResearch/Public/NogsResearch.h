// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNogsResearchCpp, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNogsResearchLoopDebugging, Log, Error);

class FNogsResearchModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
