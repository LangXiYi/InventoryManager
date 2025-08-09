// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Equipment.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Data/GridInvSys_InventoryItemInstance.h"

UInvSys_InventoryItemInstance* UGridInvSys_InventoryFragment_Equipment::EquipItemDefinition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef)
{
	check(HasAuthority())

	UGridInvSys_InventoryItemInstance* TempItemInstance = NewObject<UGridInvSys_InventoryItemInstance>(GetInventoryComponent());
	if (TempItemInstance)
	{
		TempItemInstance->SetItemDefinition(ItemDef);
		TempItemInstance->SetItemUniqueID(FGuid::NewGuid());
		EquipItemInstance(TempItemInstance);
	}
	return TempItemInstance;
}
