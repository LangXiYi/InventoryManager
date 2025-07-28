// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_PickableItems.h"

#include "Engine/ActorChannel.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "GameFramework/GameplayMessageSubsystem.h"
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

void AInvSys_PickableItems::InitItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	/*if (NewItemInstance->GetOuter() != this)
	{
		NewItemInstance = DuplicateObject(NewItemInstance, this);
	}*/
	ItemInstance = DuplicateObject(NewItemInstance, this);
	NewItemInstance->ConditionalBeginDestroy();
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRepItemInstance();
	}
}

bool AInvSys_PickableItems::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (ItemInstance && IsValid(ItemInstance))
	{
		UActorChannel::SetCurrentSubObjectOwner(this);
		for (UInvSys_InventoryItemInstance* MyInstance : ItemInstance->MyInstances)
		{
			WroteSomething |= Channel->ReplicateSubobject(MyInstance, *Bunch, *RepFlags);
		}
		WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void AInvSys_PickableItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInvSys_PickableItems, ItemInstance);
}

void AInvSys_PickableItems::OnRepItemInstance()
{
	check(ItemInstance)
	if (ItemInstance)
	{
		auto DropItemFragment = ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (DropItemFragment)
		{
			UStaticMesh* StaticMesh = DropItemFragment->DropDisplayMesh.LoadSynchronous();
			if (StaticMesh)
			{
				ItemMesh->SetStaticMesh(StaticMesh);
			}
			ItemMesh->SetRelativeLocation(DropItemFragment->DropLocationOffset);
		}
		FDropItemMessage DropItemMessage;
		DropItemMessage.DropItem = this;
		DropItemMessage.ItemInstance = ItemInstance;
		DropItemMessage.FromActor = GetOwner();
		
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(Inventory_Message_DropItem, DropItemMessage);
	}
}
