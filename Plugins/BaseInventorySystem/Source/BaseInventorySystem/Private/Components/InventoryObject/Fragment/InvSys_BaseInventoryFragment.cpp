// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_BaseInventoryFragment.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
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

void UInvSys_BaseInventoryFragment::InitInventoryFragment(/*UInvSys_BaseInventoryObject* InvObj,*/ UObject* PreEditFragment)
{
	// if (InventoryComponent)
	// {
	// 	InventoryObject = InventoryComponent->FindInventoryObject<UInvSys_BaseInventoryObject>(InventoryObjectTag);
	// }
	// check(InvObj)
	// InventoryObject = InvObj;

	// COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_BaseInventoryFragment, Priority);
}

void UInvSys_BaseInventoryFragment::RefreshInventoryFragment()
{
}

bool UInvSys_BaseInventoryFragment::ReplicateSubobjects(
	UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	UActorChannel::SetCurrentSubObjectOwner(GetInventoryComponent());
	bool bWroteSomething = false;
	// if (InventoryObject && IsValid(InventoryObject))
	// {
	// 	bWroteSomething = Channel->ReplicateSubobject(InventoryObject, *Bunch, *RepFlags);
	// }
	return bWroteSomething;
}

bool UInvSys_BaseInventoryFragment::HasAuthority() const
{
	AActor* MyOwner = GetOwner();
	check(MyOwner);
	if (MyOwner)
	{
		return MyOwner->HasAuthority();
	}
	return false;
}

ENetMode UInvSys_BaseInventoryFragment::GetNetMode() const
{
	AActor* MyOwner = GetOwner();
	check(MyOwner);
	if (MyOwner)
	{
		return MyOwner->GetNetMode();
	}
	return ENetMode::NM_Standalone;
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

FGameplayTag UInvSys_BaseInventoryFragment::GetInventoryObjectTag() const
{
	check(InventoryObjectTag.IsValid())
	return InventoryObjectTag;
}

UInvSys_BaseInventoryObject* UInvSys_BaseInventoryFragment::GetInventoryObject() const
{
	check(InventoryObject)
	return InventoryObject;
}

void UInvSys_BaseInventoryFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryFragment, InventoryObjectTag, COND_None);
}
