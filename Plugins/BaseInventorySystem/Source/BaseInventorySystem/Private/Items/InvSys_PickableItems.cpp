// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_PickableItems.h"

#include "Engine/ActorChannel.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_DropItem, "Inventory.Message.DropItem");

AInvSys_PickableItems::AInvSys_PickableItems()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(SceneComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereCollision->SetupAttachment(SceneComponent);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	bReplicates = true;
}

void AInvSys_PickableItems::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (PickableItemInstance)
	{
		if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
		{
			OnRep_PickableItemInstance();
			MarkItemDirty();
		}
	}
}

void AInvSys_PickableItems::BeginPlay()
{
	Super::BeginPlay();

}

bool AInvSys_PickableItems::PickupItem(UInvSys_InventoryComponent* InvComp, bool bIsAutoEquip)
{
	checkNoEntry();
	return false; 
}

void AInvSys_PickableItems::InitPickableItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(HasAuthority())
	check(ItemInstance)
	PickableItemInstance = ItemInstance;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_PickableItemInstance();
	}
	MarkItemDirty();
}

void AInvSys_PickableItems::MarkItemDirty()
{
	bIsDirty = true;
}

TSubclassOf<UInvSys_InventoryItemDefinition> AInvSys_PickableItems::GetItemDefinition()
{
	return PickableItemInstance->GetItemDefinition();
}

int32 AInvSys_PickableItems::GetItemStackCount() const
{
	check(PickableItemInstance)
	return PickableItemInstance->GetItemStackCount();
}

void AInvSys_PickableItems::SetItemStackCount(int32 NewStackCount)
{
	check(PickableItemInstance)
	PickableItemInstance->SetItemStackCount(NewStackCount);
	bIsDirty = true;
}

const UInvSys_InventoryItemFragment* AInvSys_PickableItems::FindFragmentByClass(
	TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if (PickableItemInstance)
	{
		return PickableItemInstance->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

bool AInvSys_PickableItems::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	
	bool bIsWrote =  Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (bIsDirty)
	{
		bIsDirty = false;
		bIsWrote |= Channel->ReplicateSubobject(PickableItemInstance, *Bunch, *RepFlags);
	}
	return bIsWrote;
}

void AInvSys_PickableItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInvSys_PickableItems, PickableItemInstance);
}

void AInvSys_PickableItems::OnRep_PickableItemInstance()
{
	auto DropItemFragment = FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DropItemFragment)
	{
		UStaticMesh* StaticMesh = DropItemFragment->DropDisplayMesh.LoadSynchronous();
		if (StaticMesh)
		{
			ItemMesh->SetStaticMesh(StaticMesh);
		}
		// ItemMesh->SetRelativeLocation(DropItemFragment->DropLocationOffset);
	}
}
