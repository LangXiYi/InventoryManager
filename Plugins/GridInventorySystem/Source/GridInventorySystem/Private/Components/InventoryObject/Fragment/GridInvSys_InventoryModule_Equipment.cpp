// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/GridInvSys_InventoryModule_Equipment.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"

UInvSys_InventoryItemInstance* UGridInvSys_InventoryModule_Equipment::EquipItemDefinition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount)
{
	check(HasAuthority())
	if (HasEquipmentItems())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Has equip other items."), __FUNCTION__)
		return nullptr;
	}
	if (ItemDef == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemDefinition is nullptr."), __FUNCTION__)
		return nullptr;
	}

	/**
	 * 物品的定义中必须包含装备片段
	 */
	auto CDO_ItemDefinition = ItemDef->GetDefaultObject<UInvSys_InventoryItemDefinition>();
	if (CDO_ItemDefinition == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, CDO_ItemDefinition is nullptr."), __FUNCTION__)
		return nullptr;
	}
	auto EquipmentFragment = CDO_ItemDefinition->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return nullptr;
	}
	if (EquipmentFragment->SupportEquipSlot.HasTagExact(InventoryTag) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 物品支持的装备槽中不包括 %s ."), __FUNCTION__, *InventoryTag.ToString())
		return nullptr;
	}

	UGridInvSys_InventoryItemInstance* TempItemInstance = NewObject<UGridInvSys_InventoryItemInstance>(GetInventoryComponent());
	if (TempItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr, %s is not valid."), __FUNCTION__, *ItemDef->GetName())
		return nullptr;
	}
	EquipmentInstance = TempItemInstance;
	EquipmentInstance->SetItemDefinition(ItemDef);
	EquipmentInstance->SetItemStackCount(StackCount);
	EquipmentInstance->SetInventoryTag(GetInventoryTag());
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemInstance();
	}
	return EquipmentInstance;
}
