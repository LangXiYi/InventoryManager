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
	DOREPLIFETIME(UInvSys_InventoryItemInstance, StackCount);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, InvComp);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, SlotTag);
}

void UInvSys_InventoryItemInstance::OnRep_StackCount()
{
	BroadcastStackChangeMessage(LastStackCount, StackCount);
	LastStackCount = StackCount;
}

void UInvSys_InventoryItemInstance::SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef)
{
	ItemDefinition = NewItemDef;
}

void UInvSys_InventoryItemInstance::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	InvComp = NewInvComp;
}

void UInvSys_InventoryItemInstance::SetItemUniqueID(FGuid Guid)
{
	ItemUniqueID = Guid;
}

void UInvSys_InventoryItemInstance::SetSlotTag(FGameplayTag Tag)
{
	SlotTag = Tag;
}

void UInvSys_InventoryItemInstance::RemoveFromInventory()
{
	// SlotTag = FGameplayTag(); //不要移除这些属性
	// InvComp = nullptr;
}

const UInvSys_InventoryItemFragment* UInvSys_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
