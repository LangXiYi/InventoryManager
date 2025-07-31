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
	/*for (int i = Entries.Num() - 1; i >= 0; --i) // 倒序删除
	{
		FInvSys_ContainerEntry& Entry = Entries[i];
		if (OwnerObject->GetNetMode() != NM_DedicatedServer)
		{
			BroadcastRemoveEntryMessage(Entry);
		}
		Entries.RemoveAt(i);
		MarkArrayDirty();
	}*/
	Entries.Empty();
	MarkArrayDirty();
}

bool FInvSys_ContainerList::RemoveEntry(UInvSys_InventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvSys_ContainerEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (OwnerObject->GetNetMode() != NM_DedicatedServer)
			{
				BroadcastRemoveEntryMessage(Entry);
			}
			
			EntryIt.RemoveCurrent(); //这里Remove后，Entry的值会同步改变
			MarkArrayDirty();
			return true;
		}
	}
	return false;
}

bool FInvSys_ContainerList::RemoveEntry(UInvSys_InventoryItemInstance* Instance, int32& OutIndex)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvSys_ContainerEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (OwnerObject->GetNetMode() != NM_DedicatedServer)
			{
				BroadcastRemoveEntryMessage(Entry);
			}
			OutIndex = EntryIt.GetIndex();
			EntryIt.RemoveCurrent(); //这里Remove后，Entry的值会同步改变
			MarkArrayDirty();
			return true;
		}
	}
	return false;
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
		BroadcastRemoveEntryMessage(Entry);
	}
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];

		/*
		 * 由于 FastArray 的属性同步优于其内部对象 ItemInstance 的属性同步，所以需要延迟一段时间
		 * 在大部分情况下，如修改数组内其他对象的属性后，添加新的对象，若新对象需要检查其他对象的属性，那么这里就会出现问题。
		 * 此时的其他对象的onRep函数未执行，客户端在执行中可能出现其他问题。
		 */

		OwnerObject->GetWorld()->GetTimerManager().SetTimerForNextTick([this, Entry]()
		{
			BroadcastAddEntryMessage(Entry);
		});
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	UE_LOG(LogInventorySystem, Error, TEXT("[%s]PostReplicatedChange"), *OwnerObject->GetOwner()->GetName())
	for (int32 Index : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		BroadcastStackChangeMessage(Entry, Entry.LastObservedCount, Entry.StackCount);
	}
}

void FInvSys_ContainerList::BroadcastAddEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryAdded.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, 0, Entry.StackCount);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = OwnerObject->GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = OwnerObject->GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(OwnerObject->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastRemoveEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryRemove.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, Entry.StackCount, 0);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = OwnerObject->GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = OwnerObject->GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(OwnerObject->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastStackChangeMessage(const FInvSys_ContainerEntry& Entry, int32 OldCount,
	int32 NewCount)
{
	FInvSys_InventoryStackChangeMessage StackChangeMessage;
	StackChangeMessage.InvComp = OwnerObject->GetInventoryComponent();
	StackChangeMessage.InventoryObjectTag = OwnerObject->GetInventoryObjectTag();
	StackChangeMessage.ItemInstance = Entry.Instance;
	StackChangeMessage.StackCount = NewCount;
	StackChangeMessage.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(OwnerObject->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_StackChanged, StackChangeMessage);
}