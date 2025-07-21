// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Data/GridInvSys_InventoryItemInstance.h"


UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
}

void UGridInvSys_GridInventoryControllerComponent::Server_TryDropItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	bool bIsSuccess = false;
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
	if (GridItemInstance)
	{
		bIsSuccess = TryDropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, GridItemInstance,
			InPos.EquipSlotTag, InPos);
	}
	UE_LOG(LogInventorySystem, Log, TEXT("[Server:%s] 尝试放置物品[%s] --> {%s}"),
		bIsSuccess ? TEXT("TRUE") : TEXT("FALSE"),
		*InItemInstance->GetItemDisplayName().ToString(), *InPos.ToString())
}
