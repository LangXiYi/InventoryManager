// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryFragment_Container::UInvSys_InventoryFragment_Container() : ContainerList(this)
{
	Priority = 20;
}

void UInvSys_InventoryFragment_Container::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	auto WarpAddItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryObjectTag() && Message.InvComp == GetInventoryComponent())
		{
			NativeOnContainerEntryAdded(Message);
		}
	};

	auto WarpRemoveItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryObjectTag() && Message.InvComp == GetInventoryComponent())
		{
			NativeOnContainerEntryRemove(Message);
		}
	};

	auto WarpItemStackChangedFunc = [this](FGameplayTag Tag, const FInvSys_InventoryStackChangeMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryObjectTag() && Message.InvComp == GetInventoryComponent())
		{
			NativeOnItemStackChange(Message);
		}
	};

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	OnAddItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_AddItem, MoveTemp(WarpAddItemFunc));

	OnRemoveItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_RemoveItem, MoveTemp(WarpRemoveItemFunc));

	OnItemStackChangedHandle = MessageSubsystem.RegisterListener<FInvSys_InventoryStackChangeMessage>(
		Inventory_Message_StackChanged, MoveTemp(WarpItemStackChangedFunc));
}

void UInvSys_InventoryFragment_Container::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	TArray<UInvSys_InventoryItemInstance*> AllItems;
	ContainerList.GetAllItems(AllItems);
	Debug_PrintContainerAllItems();
	if (AllItems.Num() > 0)
	{
		for (UInvSys_InventoryItemInstance* ItemInstance : AllItems)
		{
			FInvSys_InventoryItemChangedMessage AddItemMessage;
			AddItemMessage.InvComp = GetInventoryComponent();
			AddItemMessage.InventoryObjectTag = GetInventoryObjectTag();
			AddItemMessage.ItemInstance = ItemInstance;

			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, AddItemMessage);
		}
	}
}

void UInvSys_InventoryFragment_Container::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance,
	int32 NewStackCount)
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
	ContainerList.RemoveAll();
	MarkContainerDirty();
}

bool UInvSys_InventoryFragment_Container::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (ContainerList.RemoveEntry(InItemInstance))
	{
		MarkContainerDirty();
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
	ContainerList.GetAllItems<UInvSys_InventoryItemInstance>(OutArray);
}

void UInvSys_InventoryFragment_Container::MarkItemInstanceDirty(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	int32 Index = ContainerList.FindEntryIndex(ItemInstance);
	if (ContainerList.IsValidIndex(Index))
	{
		FInvSys_ContainerEntry& ContainerEntry = ContainerList[Index];
		ContainerList.MarkItemDirty(ContainerEntry);
		MarkContainerDirty();
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("标记物品实例为脏失败，容器内不存在 %s"), *ItemInstance->GetName())
	}
}

void UInvSys_InventoryFragment_Container::MarkContainerDirty()
{
	if (ContainerList.Num() != ContainerEntryRepKeyMap.Num() - 1)
	{
		ContainerEntryRepKeyMap.Reset();
	}
	ContainerList.MarkArrayDirty();
}

bool UInvSys_InventoryFragment_Container::KeyNeedsToReplicate(int32 ObjID, int32 RepKey)
{
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


	// DOREPLIFETIME_CONDITION()
}

bool UInvSys_InventoryFragment_Container::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bWroteSomething =  Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	// 优化执行流程，只在 ContainerList 被标记为脏时，才会触发循环执行复制！
	// 是否会影响 RepNotify？ 答案：会影响
	/*
	 * 注意：
	 * 1、服务器执行ReplicateSubobject的顺序只在初始化时有效
	 * 2、若移除子对象时未销毁这个子对象，那么在下次加入该对象时，客户端的复制顺序会按照初始化对象时的顺序执行
	 * 如何确保客户端顺序与服务器顺序一致？
	 * 等待深入研究来解决该问题！！！
	 */
	if (KeyNeedsToReplicate(0, ContainerList.ArrayReplicationKey)) // 容器内成员从下标 1 开始标记，所以数组本身可以直接使用 0 
	{
		UE_LOG(LogInventorySystem, Log, TEXT("%s::%s --> Container Size = %d"),
			*GetOwner()->GetName(), *InventoryObjectTag.ToString(), ContainerList.Num())
		for (int Index = 0; Index < ContainerList.Num(); ++Index)
		{
			const FInvSys_ContainerEntry& Entry = ContainerList[Index];
			if (KeyNeedsToReplicate(Entry.ReplicationID, Entry.ReplicationKey))
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
					TEXT("[%s:%s] %s:%s 的属性发生变化，正在同步至客户端。"), *GetOwner()->GetName(),
					*InventoryObjectTag.ToString(), *Entry.Instance->GetItemDisplayName().ToString(), *Entry.Instance.GetName())
				if (Entry.IsValid())
				{
					// 同步所有需要同步的数据
					bWroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
				}
			}
		}
	}
	return bWroteSomething;
}

void UInvSys_InventoryFragment_Container::Debug_PrintContainerAllItems()
{
	TArray<UInvSys_InventoryItemInstance*> AllItems;
	GetAllItemInstance(AllItems);

	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("= BEG =========================================================================="))
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("\t Owner: %s \t Tag: %s \t Count: %d"),
		*GetOwner()->GetName(), *GetInventoryObjectTag().ToString(), AllItems.Num())
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("--------------------------------------------------------------------------------"))
	for (UInvSys_InventoryItemInstance* ItemInstance : AllItems)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("DisplayName = %s \t Tag = %s \t Name: %s \t OuterName = %s \t"),
			*ItemInstance->GetItemDisplayName().ToString(),
			*ItemInstance->GetInventoryObjectTag().ToString(),
			*ItemInstance->GetName(),
			*ItemInstance->GetOuter()->GetName())
	}
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("= END =========================================================================="))
}