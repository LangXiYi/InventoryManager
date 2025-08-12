// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_PickableItems.h"

#include "Engine/ActorChannel.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_ItemFragment_PickUpItem.h"
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

void AInvSys_PickableItems::BeginPlay()
{
	Super::BeginPlay();
	if (ItemDefinition != nullptr)
	{
		auto DropItemFragment = FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (DropItemFragment)
		{
			UStaticMesh* StaticMesh = DropItemFragment->DropDisplayMesh.LoadSynchronous();
			if (StaticMesh)
			{
				ItemMesh->SetStaticMesh(StaticMesh);
			}
			ItemMesh->SetRelativeLocation(DropItemFragment->DropLocationOffset);
		}
	}
}

void AInvSys_PickableItems::InitItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (NewItemInstance== nullptr)
	{
		checkNoEntry();
		return;
	}
	ItemDefinition = NewItemInstance->GetItemDefinition();
	ItemStackCount = NewItemInstance->GetItemStackCount();
}

bool AInvSys_PickableItems::PickupItem(UInvSys_InventoryComponent* InvComp)
{
	checkNoEntry();
	return false; 
}

void AInvSys_PickableItems::InitPickableItems(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 NewStackCount)
{
	check(HasAuthority())
	ItemDefinition = ItemDef;
	ItemStackCount = NewStackCount;

	auto DropItemFragment = FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DropItemFragment)
	{
		UStaticMesh* StaticMesh = DropItemFragment->DropDisplayMesh.LoadSynchronous();
		if (StaticMesh)
		{
			ItemMesh->SetStaticMesh(StaticMesh);
		}
		ItemMesh->SetRelativeLocation(DropItemFragment->DropLocationOffset);
	}
}

const UInvSys_InventoryItemFragment* AInvSys_PickableItems::FindFragmentByClass(
	TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

void AInvSys_PickableItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInvSys_PickableItems, ItemDefinition);
	DOREPLIFETIME(AInvSys_PickableItems, ItemStackCount);
}