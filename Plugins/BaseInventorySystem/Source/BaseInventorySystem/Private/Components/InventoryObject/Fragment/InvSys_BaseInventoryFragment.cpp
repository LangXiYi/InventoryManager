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

bool UInvSys_BaseInventoryFragment::ReplicateSubobjects(
	UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	UActorChannel::SetCurrentSubObjectOwner(GetInventoryComponent());
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

AActor* UInvSys_BaseInventoryFragment::GetOwner() const
{
	check(Owner_Private)
	return Owner_Private;
}

UInvSys_InventoryComponent* UInvSys_BaseInventoryFragment::GetInventoryComponent() const
{
	check(InventoryComponent);
	return InventoryComponent;
}

FGameplayTag UInvSys_BaseInventoryFragment::GetInventoryTag() const
{
	check(InventoryTag.IsValid())
	return InventoryTag;
}

UInvSys_BaseInventoryObject* UInvSys_BaseInventoryFragment::GetInventoryObject() const
{
	check(InventoryObject)
	return InventoryObject;
}