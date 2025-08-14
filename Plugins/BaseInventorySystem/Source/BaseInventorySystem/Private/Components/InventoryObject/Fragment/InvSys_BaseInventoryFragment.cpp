// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_BaseInventoryFragment.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInvSys_BaseInventoryFragment::UInvSys_BaseInventoryFragment()
{
	UObject* MyOuter = GetOuter();
	if (MyOuter)
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner_Private = InventoryComponent->GetOwner();
		}
	}
}

void UInvSys_BaseInventoryFragment::MarkInventoryModuleDirty()
{
	++InventoryModuleRepKey;
}

bool UInvSys_BaseInventoryFragment::ReplicateSubobjects(
	UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	UActorChannel::SetCurrentSubObjectOwner(GetInventoryComponent());
	bPendingDormancy = Channel->KeyNeedsToReplicate(InventoryModuleID, InventoryModuleRepKey);
	return false;
}

void UInvSys_BaseInventoryFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(UInvSys_BaseInventoryFragment, InventoryObjectTag);
}

bool UInvSys_BaseInventoryFragment::HasAuthority() const
{
	check(Owner_Private);
	return Owner_Private->HasAuthority();
}

ENetMode UInvSys_BaseInventoryFragment::GetNetMode() const
{
	check(Owner_Private);
	return Owner_Private->GetNetMode();
}