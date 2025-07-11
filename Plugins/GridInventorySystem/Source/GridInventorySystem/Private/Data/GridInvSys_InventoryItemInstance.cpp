// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"

void UGridInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, ItemPosition);
}

void UGridInvSys_InventoryItemInstance::RemoveFromInventory()
{
	Super::RemoveFromInventory();

	//SetItemPosition(FGridInvSys_ItemPosition());
}
