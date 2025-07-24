// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_ContainerList.h"
#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Data/InvSys_InventoryItemInstance.h"

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_AddItem, "Inventory.Message.AddItem");
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_RemoveItem, "Inventory.Message.RemoveItem");

/*const FInvSys_ContainerEntry& FInvSys_ContainerList::AddEntry(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
	int32 StackCount)
{
	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());
		
	FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UInvSys_InventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Instance->SetItemDefinition(ItemDef);
	for (const UInvSys_InventoryItemFragment* Fragment : GetDefault<UInvSys_InventoryItemDefinition>(ItemDef)->GetFragments())
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	NewEntry.ItemUniqueID = FGuid::NewGuid();

	MarkItemDirty(NewEntry);
	return NewEntry;
}*/

FString FInvSys_ContainerEntry::GetDebugString() const
{
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDefinition();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

void FInvSys_ContainerList::BroadcastAddEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryAdded.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, 0, Entry.StackCount);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(OwnerObject->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

void FInvSys_ContainerList::BroadcastRemoveEntryMessage(const FInvSys_ContainerEntry& Entry)
{
	OnContainerEntryRemove.ExecuteIfBound(Entry, false);
	// BroadcastStackChangeMessage(Entry, Entry.StackCount, 0);

	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.ItemInstance = Entry.Instance;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(OwnerObject->GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

// bool FInvSys_ContainerList::AddEntry(UInvSys_InventoryItemInstance* Instance)
// {
// 	if (Instance == nullptr)
// 	{
// 		return false;
// 	}
// 	
// 	FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
// 	NewEntry.Instance = Instance;
//
// 	MarkItemDirty(NewEntry);
//
// 	check(OwnerObject)
// 	if (OwnerObject && OwnerObject->GetNetMode() != NM_DedicatedServer)
// 	{
// 		BroadcastAddEntryMessage(NewEntry);
// 	}
// 	return true;
// }

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

/*bool FInvSys_ContainerList::UpdateEntryStackCount(UInvSys_InventoryItemInstance* Instance, int32 NewCount)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInvSys_ContainerEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			Entry.StackCount = NewCount;
			MarkItemDirty(Entry);
			if (OwnerObject->GetNetMode() != NM_DedicatedServer)
			{
				BroadcastStackChangeMessage(Entry, Entry.LastObservedCount, Entry.StackCount);
			}
			Entry.LastObservedCount = Entry.StackCount;
			return true;
		}
	}
	return false;
}*/

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
		BroadcastAddEntryMessage(Entry);
	}
}

void FInvSys_ContainerList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FInvSys_ContainerEntry& Entry = Entries[Index];
		//BroadcastStackChangeMessage(Entry, Entry.LastObservedCount, Entry.StackCount);
	}
}
