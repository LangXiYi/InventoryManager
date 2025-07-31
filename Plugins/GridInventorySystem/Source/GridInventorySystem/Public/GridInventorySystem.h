// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_ItemPositionChanged)

class FGridInventorySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
