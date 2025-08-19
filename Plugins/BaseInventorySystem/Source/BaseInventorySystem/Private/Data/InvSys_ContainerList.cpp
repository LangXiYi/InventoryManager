// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_ContainerList.h"

#include "BaseInventorySystem.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"


void FInvSys_ContainerEntry::PreReplicatedRemove(const FFastArraySerializer& InArraySerializer)
{
	// if (Instance == nullptr)
	// {
	// 	UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Instance is nullptr."), __FUNCTION__)
	// 	return;
	// }
	// Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
	// // UE_LOG(LogInventorySystem, Log, TEXT("PreReplicatedRemove: %s "), *Instance->GetName())
	// Instance->PreReplicatedRemove();
}

void FInvSys_ContainerEntry::PostReplicatedAdd(const FFastArraySerializer& InArraySerializer)
{
	// if (Instance == nullptr)
	// {
	// 	UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Instance is nullptr."), __FUNCTION__)
	// 	return;
	// }
	// Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
	// // UE_LOG(LogInventorySystem, Log, TEXT("PostReplicatedAdd: %s "), *Instance->GetName())
}

void FInvSys_ContainerEntry::PostReplicatedChange(const FFastArraySerializer& InArraySerializer)
{
	// if (Instance == nullptr)
	// {
	// 	UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Instance is nullptr."), __FUNCTION__)
	// 	return;
	// }
	// Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
	// // UE_LOG(LogInventorySystem, Log, TEXT("PostReplicatedChange: %s "), *Instance->GetName())
}

FString FInvSys_ContainerEntry::GetDebugString() const
{
	if (Instance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Instance is nullptr."), __FUNCTION__)
		return "None";
	}
	return FString::Printf(TEXT("%s -- %s"), *GetNameSafe(Instance), *Instance->GetItemDisplayName().ToString());
}

void FInvSys_ContainerList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for (int32 RemovedIndex : RemovedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[RemovedIndex];
		if (Entry.Instance && IsValid(Entry.Instance))
		{
			Entry.Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
			Entry.Instance->PreReplicatedRemove();
		}
	}
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 AddIndex : AddedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[AddIndex];
		if (Entry.Instance && IsValid(Entry.Instance))
		{
			Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
		}
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[ChangedIndex];
		if (Entry.Instance && IsValid(Entry.Instance))
		{
			Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
		}
	}
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
			EntryIt.RemoveCurrent(); //这里Remove后，Entry的值会同步改变
			MarkArrayDirty();
			return true;
		}
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例在容器内不存在."), __FUNCTION__)
	return false;
}

void FInvSys_ContainerList::GetAllItems(TArray<UInvSys_InventoryItemInstance*>& OutArray) const
{
	// bug::物品对象的指针存在，但目标位置的对象缺并未创建
	OutArray.Reserve(Entries.Num());
	for (const FInvSys_ContainerEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr && IsValid(Entry.Instance) && Entry.Instance->IsA<UInvSys_InventoryItemInstance>())
		{
			OutArray.Add(Entry.Instance);
		}
	}
}

void FInvSys_ContainerList::FindItemInstance(const TSubclassOf<UInvSys_InventoryItemDefinition>& ItemDefinition,
	TArray<UInvSys_InventoryItemInstance*>& OutArray) const
{
	for (FInvSys_ContainerEntry Entry : Entries)
	{
		UInvSys_InventoryItemInstance* ItemInstance = Entry.Instance;
		if (ItemInstance != nullptr)
		{
			if (ItemDefinition == ItemInstance->GetItemDefinition())
			{
				OutArray.Add(ItemInstance);
			}
		}
	}
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
	// UE_LOG(LogInventorySystem, Warning, TEXT("传入的物品实例 [%s] 在当前容器 [%s] 内不存在"),
	// 	*ItemInstance->GetItemDisplayName().ToString(), *GetInventoryObjectTag().ToString())
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
	return InventoryFragment->GetInventoryTag();
}
