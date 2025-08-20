// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryModule.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryModule::UInvSys_InventoryModule()
{
	InventoryComponent = GetTypedOuter<UInvSys_InventoryComponent>();
	if (InventoryComponent)
	{
		Owner_Private = InventoryComponent->GetOwner();
	}
}

void UInvSys_InventoryModule::MarkInventoryModuleDirty()
{
	++InventoryModuleRepKey;
}

bool UInvSys_InventoryModule::ReplicateSubobjects(
	UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	UActorChannel::SetCurrentSubObjectOwner(GetInventoryComponent());
	bPendingDormancy = Channel->KeyNeedsToReplicate(InventoryModuleID, InventoryModuleRepKey);
	return false;
}

void UInvSys_InventoryModule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(UInvSys_BaseInventoryFragment, InventoryObjectTag);
}

bool UInvSys_InventoryModule::HasAuthority() const
{
	check(Owner_Private);
	return Owner_Private->HasAuthority();
}

ENetMode UInvSys_InventoryModule::GetNetMode() const
{
	check(Owner_Private);
	return Owner_Private->GetNetMode();
}