// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Components/InvSys_InventoryComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryFragment_Equipment::UInvSys_InventoryFragment_Equipment()
{
	Priority = 10;
}

void UInvSys_InventoryFragment_Equipment::RefreshInventoryFragment()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("正在刷新装备片段[%s]"), *GetInventoryObjectTag().ToString())
	if (ItemInstance)
	{
		BroadcastEquipItemInstance(ItemInstance);
	}
	else
	{
		BroadcastUnEquipItemInstance();
	}
}

void UInvSys_InventoryFragment_Equipment::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryFragment_Equipment, SupportEquipTag);
}

UInvSys_InventoryItemInstance* UInvSys_InventoryFragment_Equipment::EquipItemDefinition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef)
{
	if (HasAuthority())
	{
		UInvSys_InventoryItemInstance* TempItemInstance = NewObject<UInvSys_InventoryItemInstance>(GetInventoryComponent());
		if (TempItemInstance)
		{
			TempItemInstance->SetItemDefinition(ItemDef);
			TempItemInstance->SetItemUniqueID(FGuid::NewGuid());
			EquipItemInstance(TempItemInstance);
		}
		return TempItemInstance;
	}
	return nullptr;
}

void UInvSys_InventoryFragment_Equipment::EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (HasAuthority())
	{
		if (NewItemInstance)
		{
			ItemInstance = NewItemInstance;
			// ItemInstance->SetInventoryComponent(GetInventoryComponent());
			ItemInstance->SetSlotTag(GetInventoryObjectTag());
			if (GetNetMode() != NM_DedicatedServer)
			{
				OnRep_ItemInstance();
			}
		}
	}
}

bool UInvSys_InventoryFragment_Equipment::UnEquipItemInstance()
{
	if (HasAuthority())
	{
		// EquipItemInstance->RemoveFromInventory();
		ItemInstance = nullptr;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_ItemInstance();
		}
		return true;
	}
	return false;
}

bool UInvSys_InventoryFragment_Equipment::HasEquipmentItems() const
{
	return ItemInstance && IsValid(ItemInstance);
}

UInvSys_InventoryItemInstance* UInvSys_InventoryFragment_Equipment::GetEquipItemInstance() const
{
	return ItemInstance;
}

bool UInvSys_InventoryFragment_Equipment::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                              FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (ItemInstance && IsValid(ItemInstance))
	{
		bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UInvSys_InventoryFragment_Equipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_InventoryFragment_Equipment, ItemInstance, COND_None);
}

void UInvSys_InventoryFragment_Equipment::BroadcastEquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld)
	{
		FInvSys_EquipItemInstanceMessage EquipItemInstanceMessage;
		EquipItemInstanceMessage.InvComp = GetInventoryComponent();
		EquipItemInstanceMessage.InventoryObjectTag = GetInventoryObjectTag();
		EquipItemInstanceMessage.ItemInstance = NewItemInstance;

		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(MyWorld);
		GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_EquipItem, EquipItemInstanceMessage);
	}
}

void UInvSys_InventoryFragment_Equipment::BroadcastUnEquipItemInstance()
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld)
	{
		FInvSys_EquipItemInstanceMessage UnEquipItemInstanceMessage;
		UnEquipItemInstanceMessage.InvComp = GetInventoryComponent();
		UnEquipItemInstanceMessage.InventoryObjectTag = GetInventoryObjectTag();
		UnEquipItemInstanceMessage.ItemInstance = nullptr;

		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(MyWorld);
		GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_UnEquipItem, UnEquipItemInstanceMessage);
	}
}

void UInvSys_InventoryFragment_Equipment::OnRep_ItemInstance()
{
	// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("[%s] OnRep Equip ItemInstance"), HasAuthority() ? TEXT("Server"):TEXT("Client"));
	if (ItemInstance)
	{
		BroadcastEquipItemInstance(ItemInstance);
	}
	else
	{
		BroadcastUnEquipItemInstance();
	}
	LastItemInstance = ItemInstance;
}
