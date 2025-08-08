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
		/*if (OwnerObject->GetNetMode() != NM_DedicatedServer)
		{
			BroadcastRemoveEntryMessage(Entry);
		}*/
		Entry.Instance->RemoveFromInventory();
	}
	Entries.Reset();
	MarkArrayDirty();
}

bool FInvSys_ContainerList::RemoveEntry(UInvSys_InventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvSys_ContainerEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (InventoryFragment->GetNetMode() != NM_DedicatedServer)
			{
				// BroadcastRemoveEntryMessage(Entry);
				Instance->BroadcastRemoveItemInstanceMessage();
			}
			Entry.Instance->RemoveFromInventory();
			EntryIt.RemoveCurrent(); //这里Remove后，Entry的值会同步改变
			MarkArrayDirty();
			return true;
		}
	}
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

void FInvSys_ContainerList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
		// UE_LOG(LogInventorySystem, Log, TEXT("Replicate PreRemove: %s ------- %s"),
		// 	*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
		Entry.Instance->PreReplicatedRemove();
	}
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
		// UE_LOG(LogInventorySystem, Log, TEXT("Replicate PostAdd: %s ------- %s"),
		// 	*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		Entry.Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
		// UE_LOG(LogInventorySystem, Log, TEXT("Replicate PostChange: %s ------- %s"),
		// 	*Entry.Instance->GetName(), *InventoryFragment->GetInventoryObjectTag().ToString())
	}
}