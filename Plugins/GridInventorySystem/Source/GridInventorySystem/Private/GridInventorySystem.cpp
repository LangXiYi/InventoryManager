// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridInventorySystem.h"

#include "BaseInventorySystem.h"

DEFINE_LOG_CATEGORY(LogInventorySystem);

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_AddItem, "Inventory.Message.AddItem")
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_RemoveItem, "Inventory.Message.RemoveItem")
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_StackChanged, "Inventory.Message.StackChanged")

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_EquipItem, "Inventory.Message.EquipItem");
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_UnEquipItem, "Inventory.Message.UnEquipItem");

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_DragItem, "Inventory.Message.DragItem");

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_ItemPositionChanged, "Inventory.Message.ItemPositionChanged");

#define LOCTEXT_NAMESPACE "FGridInventorySystemModule"

void FGridInventorySystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FGridInventorySystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGridInventorySystemModule, GridInventorySystem)