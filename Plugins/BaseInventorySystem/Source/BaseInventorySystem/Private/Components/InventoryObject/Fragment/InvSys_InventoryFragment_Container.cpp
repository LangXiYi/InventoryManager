// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryFragment_Container::UInvSys_InventoryFragment_Container() : ContainerList(this)
{
	Priority = 20;
}

void UInvSys_InventoryFragment_Container::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	TArray<UInvSys_InventoryItemInstance*> AllItems;
	ContainerList.GetAllItems(AllItems);
	if (AllItems.Num() > 0)
	{
		for (UInvSys_InventoryItemInstance* ItemInstance : AllItems)
		{
			FInvSys_InventoryItemChangedMessage AddItemMessage;
			AddItemMessage.InvComp = GetInventoryComponent();
			AddItemMessage.InventoryObjectTag = GetInventoryTag();
			AddItemMessage.ItemInstance = ItemInstance;

			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, AddItemMessage);
		}
	}
}

int32 UInvSys_InventoryFragment_Container::FindStackableItemInstances(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, TArray<UInvSys_InventoryItemInstance*>& StackableItems)
{
	if (ItemDef == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falid, ItemDefinition is nullptr"), __FUNCTION__)
		return 0;
	}
	auto CDO_ItemDefinition = ItemDef->GetDefaultObject<UInvSys_InventoryItemDefinition>();
	check(CDO_ItemDefinition)
	auto BaseItemFragment = CDO_ItemDefinition->FindFragmentByClass<UInvSys_ItemFragment_BaseItem>();
	if (BaseItemFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falid, BaseItemFragment is nullptr"), __FUNCTION__)
		return 0;
	}

	int32 RemainCount = 0;
	for (int i = 0; i < ContainerList.Num(); ++i)
	{
		if (ContainerList[i].Instance == nullptr)
		{
			UE_LOG(LogInventorySystem, Warning, TEXT("ContainerList[%d].Instance is nullptr"), i);
			continue;
		}
		// 物品类型相同，且当前堆叠数量未达到最大值
		if (ItemDef == ContainerList[i].Instance->GetItemDefinition())
		{
			int32 StackCount = ContainerList[i].Instance->GetItemStackCount();
		    if (StackCount < BaseItemFragment->MaxStackCount)
		    {
			    StackableItems.Add(ContainerList[i].Instance);
		    	RemainCount += BaseItemFragment->MaxStackCount - StackCount;
		    }
		}
	}
	return RemainCount;
}

int32 UInvSys_InventoryFragment_Container::FindStackableItemInstances(TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance,
	TArray<UInvSys_InventoryItemInstance*>& StackableItems)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falid, ItemInstance is nullptr"), __FUNCTION__)
		return 0;
	}
	return FindStackableItemInstances(ItemInstance->GetItemDefinition(), StackableItems);
}

void UInvSys_InventoryFragment_Container::UpdateItemStackCount(
	UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount)
{
	if (ContainsItem(ItemInstance))
	{
		ItemInstance->SetItemStackCount(NewStackCount);
		MarkItemInstanceDirty(ItemInstance);		
	}
}

void UInvSys_InventoryFragment_Container::UpdateItemInstanceDragState(UInvSys_InventoryItemInstance* ItemInstance,
                                                                      bool NewState)
{
	if (ContainsItem(ItemInstance))
	{
		ItemInstance->SetIsDraggingItem(NewState);
		MarkItemInstanceDirty(ItemInstance);		
	}
}

void UInvSys_InventoryFragment_Container::RemoveAllItemInstance()
{
	int32 Count = ContainerList.Num();
	for (int i = 0; i < Count; ++i)
	{
		OnContainerPreRemove(ContainerList[i].Instance);
	}
	ContainerList.RemoveAll();
	MarkInventoryModuleDirty();
}

bool UInvSys_InventoryFragment_Container::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (ContainerList.RemoveEntry(InItemInstance))
	{
		MarkInventoryModuleDirty();
		OnContainerPreRemove(InItemInstance);
		return true;
	}
	return false;
}

bool UInvSys_InventoryFragment_Container::ContainsItem(UInvSys_InventoryItemInstance* ItemInstance) const
{
	return ContainerList.Contains(ItemInstance);
}

void UInvSys_InventoryFragment_Container::GetAllItemInstance(TArray<UInvSys_InventoryItemInstance*>& OutArray) const
{
	ContainerList.GetAllItems(OutArray);
}

void UInvSys_InventoryFragment_Container::MarkItemInstanceDirty(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	int32 Index = ContainerList.FindEntryIndex(ItemInstance);
	if (ContainerList.IsValidIndex(Index))
	{
		FInvSys_ContainerEntry& ContainerEntry = ContainerList[Index];
		ContainerList.MarkItemDirty(ContainerEntry);
		MarkInventoryModuleDirty();
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("标记物品实例为脏失败，容器内不存在 %s"), *ItemInstance->GetName())
	}
}

void UInvSys_InventoryFragment_Container::MarkInventoryModuleDirty()
{
	Super::MarkInventoryModuleDirty();
	if (ContainerList.Num() != ContainerEntryRepKeyMap.Num() - 1)
	{
		ContainerEntryRepKeyMap.Reset();
	}
	ContainerList.MarkArrayDirty();
}

bool UInvSys_InventoryFragment_Container::KeyNeedsToReplicate(int32 ObjID, int32 RepKey)
{
	if (bPendingDormancy)
	{
		// Return true to keep the channel from being stuck unable to go dormant until the key changes
		return true;
	}
	int32 &MapKey = ContainerEntryRepKeyMap.FindOrAdd(ObjID, INDEX_NONE);
	if (MapKey == RepKey)
	{
		return false;
	}
	MapKey = RepKey;
	return true;
}

void UInvSys_InventoryFragment_Container::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInvSys_InventoryFragment_Container, ContainerList, COND_None)
}

bool UInvSys_InventoryFragment_Container::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bWroteSomething =  Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	/*
	 * NOTE:服务器执行 ReplicateSubobjects 的顺序只在初始化时有效
	 */
	if (KeyNeedsToReplicate(0, ContainerList.ArrayReplicationKey)) 
	{
		for (int Index = 0; Index < ContainerList.Num(); ++Index)
		{
			const FInvSys_ContainerEntry& Entry = ContainerList[Index];
			if (Entry.IsValid() ==  false)
			{
				UE_LOG(LogInventorySystem, Warning, TEXT("ContainerEntry is not valid --> InventoryTag: %s, Index: %d"),
					*InventoryTag.ToString(), Index)
				continue;
			}
			if (KeyNeedsToReplicate(Entry.ReplicationID, Entry.ReplicationKey))
			{
				// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
				// 	TEXT("[%s:%s] %s:%s 的属性发生变化，正在同步至客户端。"), *GetOwner()->GetName(),
				// 	*InventoryTag.ToString(), *Entry.Instance->GetItemDisplayName().ToString(), *Entry.Instance->GetName())
				bWroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
			}
		}
	}
	return bWroteSomething;
}