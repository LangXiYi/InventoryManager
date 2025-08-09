// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_ContainerList.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Data/InvSys_InventoryItemInstance.h"

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

void FInvSys_ContainerList::RemoveAt(int32 Index)
{
	if (Entries.IsValidIndex(Index))
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->RemoveFromInventory();
		Entries.RemoveAt(Index);
		MarkArrayDirty();
	}
}

UInvSys_InventoryItemInstance* FInvSys_ContainerList::FindItem(FGuid ItemUniqueID) const
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

int32 FInvSys_ContainerList::FindEntryIndex(UInvSys_InventoryItemInstance* ItemInstance)
{
	for (int i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].Instance == ItemInstance)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void FInvSys_ContainerList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
		UE_LOG(LogInventorySystem, Log, TEXT("Replicate PreRemove: %s ------- %s"),
			*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
		Entry.Instance->PreReplicatedRemove();
	}
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
		UE_LOG(LogInventorySystem, Log, TEXT("Replicate PostAdd: %s ------- %s"),
			*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
		UE_LOG(LogInventorySystem, Log, TEXT("Replicate PostChange: %s ------- %s"),
			*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
	}
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
