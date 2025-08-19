// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"

#define MAX_INVENTORY_MODULE 50

#if WITH_EDITOR
	// A debugging aid set when we switch out different play worlds during Play In Editor / PIE
	extern ENGINE_API FString GPlayInEditorContextString;
#endif

BASEINVENTORYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Log, All);

BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_AddItem)
BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_RemoveItem)
BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_DragItem)
BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_StackChanged)
BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_EquipItem)
BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_UnEquipItem)

BASEINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_QuickBarChanged)

#define PRINT_INVENTORY_SYSTEM_LOG UE::InventorySystem::ShouldLogMessages == 1

namespace UE
{
	namespace InventorySystem
	{
		static int32 ShouldLogMessages = 1;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("InventorySystem.LogMessages"),
			ShouldLogMessages,
			TEXT("是否打印库存日志?"));
	}
}

class FBaseInventorySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
