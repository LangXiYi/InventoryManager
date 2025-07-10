// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemInstance.h"

#include "Data/InvSys_InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryItemInstance::UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer)
{
}

void UInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemUniqueID);
}

void UInvSys_InventoryItemInstance::SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef)
{
	ItemDefinition = NewItemDef;
}

void UInvSys_InventoryItemInstance::SetItemUniqueID(FGuid Guid)
{
	ItemUniqueID = Guid;
}


const UInvSys_InventoryItemFragment* UInvSys_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
