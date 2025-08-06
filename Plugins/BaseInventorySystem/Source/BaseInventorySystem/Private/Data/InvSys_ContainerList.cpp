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
	/*
	 * Remove 比较特殊需要额外处理，因为 Entries 在移除了对象后，外部的 OnRep 无法处理这个被移除的数据
	 * 只能在 FastArray 的 PreRemove 中处理这部分逻辑，但由于 PreRemove 的执行顺序比 对象本身的属性复制要更早执行，所以需要延迟一帧。
	 * 注意：RemovedIndices的内容只在当前帧有效，所以不能在下一帧直接使用这个数组的值
	 * 以下为不延迟的执行顺序 与 延时后的执行顺序
	 * PreReplicatedRemove <-- 1 --> OnRep_PropertyName
	 * OnRep_PropertyName  <-- 2 --> OnRepContainerList
	 * OnRepContainerList  <-- 3 --> PreReplicatedRemove
	 * 注意2：PreRemove执行在所有复制行为发生之前
	 */

	/*for (int32 Index : RemovedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		BroadcastRemoveEntryMessage(Entry);

		// UInvSys_InventoryItemInstance* ItemInstance = Entry.Instance;
		// InventoryFragment->GetWorld()->GetTimerManager().SetTimerForNextTick([this, ItemInstance]()
		// {
		// 	if (ItemInstance && ItemInstance->GetIsReadyReplicatedProperties())
		// 	{
		// 		ItemInstance->ReplicatedProperties();
		// 	}
		// });
	}*/
}

void FInvSys_ContainerList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	/*for (int32 Index : AddedIndices)
	{

		FInvSys_ContainerEntry& Entry = Entries[Index];
		if (Entry.StackCount == 1)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("PostReplicatedAdd"))
		}
		// InventoryFragment->GetWorld()->GetTimerManager().SetTimerForNextTick([this, Entry]()
		{
			BroadcastAddEntryMessage(Entry);
		}//);
	}*/
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	// Changed 函数在其内部属性同步之前执行？
	/*for (int32 Index : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		UE_LOG(LogInventorySystem, Warning, TEXT("PostReplicatedChange"))
	}*/
}