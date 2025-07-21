// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryControllerComponent.h"

#include "Components/InvSys_InventoryComponent.h"


UInvSys_InventoryControllerComponent::UInvSys_InventoryControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInvSys_InventoryControllerComponent::Server_TryDragItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InvComp)
	check(InItemInstance)
	bIsSuccessDragItem = InvComp ? InvComp->TryDragItemInstance(InItemInstance) : false;
	DraggingItemInstance = bIsSuccessDragItem ? InItemInstance : nullptr;
}

bool UInvSys_InventoryControllerComponent::HasAuthority() const
{
	AActor* Owner = GetOwner();
	check(Owner);
	return Owner->HasAuthority();
}

FTimerManager& UInvSys_InventoryControllerComponent::GetWorldTimerManager() const
{
	AActor* Owner = GetOwner();
	check(Owner);
	return Owner->GetWorldTimerManager();
}
