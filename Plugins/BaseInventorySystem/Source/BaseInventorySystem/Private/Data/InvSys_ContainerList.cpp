// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_ContainerList.h"

#include "BaseInventorySystem.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"


FString FInvSys_ContainerEntry::GetDebugString() const
{
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDefinition();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

void FInvSys_ContainerList::RemoveAll()
{
	for (int i = Entries.Num() - 1; i >= 0; --i) // 倒序删除
	{
		FInvSys_ContainerEntry& Entry = Entries[i];
		if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
		{
			BroadcastRemoveEntryMessage(Entry.Instance);
		}
	}
	Entries.Reset();
	MarkArrayDirty();
}

bool FInvSys_ContainerList::RemoveEntry(UInvSys_InventoryItemInstance* Instance)
{
	if (Instance == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvSys_ContainerEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
			{
				BroadcastRemoveEntryMessage(Instance);
			}
			Entry.Instance->RemoveFromInventory();
			EntryIt.RemoveCurrent(); //这里Remove后，Entry的值会同步改变
			MarkArrayDirty();
			return true;
		}
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例在容器内不存在."), __FUNCTION__)
	return false;
}

UInvSys_InventoryItemInstance* FInvSys_ContainerList::FindItemInstance(FGuid ItemUniqueID) const
{
	for (FInvSys_ContainerEntry Entry : Entries)
	{
		UInvSys_InventoryItemInstance* ItemInstance = Entry.Instance;
		if (ItemInstance != nullptr)
		{
			if (ItemUniqueID == ItemInstance->GetItemUniqueID())
			{
				return ItemInstance;
			}
		}
	}
	return nullptr;
}

UInvSys_InventoryItemInstance* FInvSys_ContainerList::FindItemInstance(
	const TSubclassOf<UInvSys_InventoryItemDefinition>& ItemDefinition) const
{
	for (FInvSys_ContainerEntry Entry : Entries)
	{
		UInvSys_InventoryItemInstance* ItemInstance = Entry.Instance;
		if (ItemInstance != nullptr)
		{
			if (ItemDefinition == ItemInstance->GetItemDefinition())
			{
				return ItemInstance;
			}
		}
	}
	return nullptr;
}

int32 FInvSys_ContainerList::FindEntryIndex(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance)
	{
		for (int i = 0; i < Entries.Num(); ++i)
		{
			if (Entries[i].Instance == ItemInstance)
			{
				return i;
			}
		}
	}
	return INDEX_NONE;
}

bool FInvSys_ContainerList::Contains(UInvSys_InventoryItemInstance* ItemInstance) const
{
	for (FInvSys_ContainerEntry Entry : Entries)
	{
		if (Entry.Instance == ItemInstance)
		{
			return true;
		}
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("传入的物品实例 [%s] 在当前容器 [%s] 内不存在"),
		*ItemInstance->GetItemDisplayName().ToString(), *GetInventoryObjectTag().ToString())
	return false;
}

void FInvSys_ContainerList::BroadcastRemoveEntryMessage(UInvSys_InventoryItemInstance* ItemInstance) const
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = ItemInstance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(InventoryFragment->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastAddEntryMessage(UInvSys_InventoryItemInstance* ItemInstance) const
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = ItemInstance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

UWorld* FInvSys_ContainerList::GetWorld() const
{
	check(InventoryFragment)
	return InventoryFragment->GetWorld();
}

bool FInvSys_ContainerList::HasAuthority() const
{
	check(InventoryFragment)
	return InventoryFragment->HasAuthority();
}

ENetMode FInvSys_ContainerList::GetNetMode() const
{
	check(InventoryFragment)
	return InventoryFragment->GetNetMode();
}

UInvSys_InventoryComponent* FInvSys_ContainerList::GetInventoryComponent() const
{
	check(InventoryFragment)
	return InventoryFragment->GetInventoryComponent();
}

FGameplayTag FInvSys_ContainerList::GetInventoryObjectTag() const
{
	check(InventoryFragment)
	return InventoryFragment->GetInventoryObjectTag();
}
