// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

CRIMITEMSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogCrimItemSystem, Log, All);

class FCrimItemSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
