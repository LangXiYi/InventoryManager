// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"

void UGridInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, ItemPosition);
	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, LastItemPosition);
}

void UGridInvSys_InventoryItemInstance::RemoveFromInventory()
{
	Super::RemoveFromInventory();

	//SetItemPosition(FGridInvSys_ItemPosition());
}

void UGridInvSys_InventoryItemInstance::OnRep_ItemPosition()
{
	// if (bIsFirstRepItemPosition) //延迟第一次同步，确保last始终在上一次的记录。
	// {
	// 	bIsFirstRepItemPosition = false;
	// 	NewTempItemPosition = ItemPosition;
	// }
	// else
	// {
	// 	LastItemPosition = NewTempItemPosition;
	// 	NewTempItemPosition = ItemPosition;
	// 	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("LastItemPosition ===> %s"), *LastItemPosition.ToString())
	// }
	// LastItemPosition = ItemPosition;
}