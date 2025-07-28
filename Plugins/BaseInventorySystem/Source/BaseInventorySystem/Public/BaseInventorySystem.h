// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Log, All);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_AddItem)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_RemoveItem)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_StackChanged)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_EquipItem);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Message_UnEquipItem);

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
