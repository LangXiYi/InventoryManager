// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseInventorySystem.h"

DEFINE_LOG_CATEGORY(LogInventorySystem);

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_AddItem, "Inventory.Message.AddItem")
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_RemoveItem, "Inventory.Message.RemoveItem")
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_StackChanged, "Inventory.Message.StackChanged")

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_EquipItem, "Inventory.Message.EquipItem");
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_UnEquipItem, "Inventory.Message.UnEquipItem");

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_DragItem, "Inventory.Message.DragItem");
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_QuickBarChanged, "Inventory.Message.QuickBarChanged");

#define LOCTEXT_NAMESPACE "FBaseInventorySystemModule"

void FBaseInventorySystemModule::StartupModule()
{

}

void FBaseInventorySystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBaseInventorySystemModule, BaseInventorySystem)


