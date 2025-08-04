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
			Entry.Instance->RemoveFromInventory();
			if (InventoryFragment->GetNetMode() != NM_DedicatedServer)
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
	/*
	 * Remove 比较特殊需要额外处理，因为 Entries 在移除了对象后，外部的 OnRep 无法处理这个被移除的数据
	 * 只能在 FastArray 的 PreRemove 中处理这部分逻辑，但由于 PreRemove 的执行顺序比 对象本身的属性复制要更早执行，所以需要延迟一帧。
	 * 注意：RemovedIndices的内容只在当前帧有效，所以不能在下一帧直接使用这个数组的值
	 * 以下为不延迟的执行顺序 与 延时后的执行顺序
	 * PreReplicatedRemove <-- 1 --> OnRep_PropertyName
	 * OnRep_PropertyName  <-- 2 --> OnRepContainerList
	 * OnRepContainerList  <-- 3 --> PreReplicatedRemove
	 */

	for (int32 Index : RemovedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		BroadcastRemoveEntryMessage(Entry);

		UInvSys_InventoryItemInstance* ItemInstance = Entry.Instance;
		InventoryFragment->GetWorld()->GetTimerManager().SetTimerForNextTick([this, ItemInstance]()
		{
			if (ItemInstance && ItemInstance->GetIsReadyReplicatedProperties())
			{
				ItemInstance->ReplicatedProperties();
			}
		});
	}
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		// if (Entry.Instance && Entry.Instance->GetIsReadyReplicatedProperties())
		// {
		// 	Entry.Instance->ReplicatedProperties();
		// }
		BroadcastAddEntryMessage(Entry);
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	// Changed 函数在其内部属性同步之前执行？
}

void FInvSys_ContainerList::BroadcastAddEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryAdded.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, 0, Entry.StackCount);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = InventoryFragment->GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = InventoryFragment->GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(InventoryFragment->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastRemoveEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryRemove.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, Entry.StackCount, 0);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = InventoryFragment->GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = InventoryFragment->GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(InventoryFragment->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastStackChangeMessage(const FInvSys_ContainerEntry& Entry, int32 OldCount,
	int32 NewCount)
{
	FInvSys_InventoryStackChangeMessage StackChangeMessage;
	StackChangeMessage.InvComp = InventoryFragment->GetInventoryComponent();
	StackChangeMessage.InventoryObjectTag = InventoryFragment->GetInventoryObjectTag();
	StackChangeMessage.ItemInstance = Entry.Instance;
	StackChangeMessage.StackCount = NewCount;
	StackChangeMessage.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(InventoryFragment->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_StackChanged, StackChangeMessage);
}