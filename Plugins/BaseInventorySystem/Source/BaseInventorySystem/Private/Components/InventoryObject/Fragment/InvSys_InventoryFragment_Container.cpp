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

void UInvSys_InventoryFragment_Container::InitInventoryFragment(/*UInvSys_BaseInventoryObject* InvObj,*/
	UObject* PreEditFragment)
{
	Super::InitInventoryFragment(/*InvObj, */PreEditFragment);
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

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	OnAddItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_AddItem, MoveTemp(WarpAddItemFunc));

	OnRemoveItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_RemoveItem, MoveTemp(WarpRemoveItemFunc));
}

void UInvSys_InventoryFragment_Container::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	TArray<UInvSys_InventoryItemInstance*> AllItems = ContainerList.GetAllItems();
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("正在刷新容器片段, 该片段内物品数量 = %d"), AllItems.Num())
	if (AllItems.Num() > 0)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("============ 物品数据============"))
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("Name \t Stack Count \t Inv Obj Tags"))
		for (UInvSys_InventoryItemInstance* ItemInstance : AllItems)
		{
			FInvSys_InventoryItemChangedMessage AddItemMessage;
			AddItemMessage.InvComp = GetInventoryComponent();
			AddItemMessage.InventoryObjectTag = GetInventoryObjectTag();
			AddItemMessage.ItemInstance = ItemInstance;

			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("%s \t %d \t %s"),
				*ItemInstance->GetItemDisplayName().ToString(), ItemInstance->GetStackCount(), *ItemInstance->GetSlotTag().ToString())

			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, AddItemMessage);
		}
	}
}

void UInvSys_InventoryFragment_Container::RemoveAllItemInstance()
{
	ContainerList.RemoveAll();
}

bool UInvSys_InventoryFragment_Container::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		return ContainerList.RemoveEntry(InItemInstance);
	}
	return false;
}

bool UInvSys_InventoryFragment_Container::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance,
	int32 NewStackCount)
{
	return false;
}

bool UInvSys_InventoryFragment_Container::ContainsItem(UInvSys_InventoryItemInstance* ItemInstance) const
{
	return ContainerList.Contains(ItemInstance);
}

TArray<UInvSys_InventoryItemInstance*> UInvSys_InventoryFragment_Container::GetAllItemInstance()
{
	return ContainerList.GetAllItems<UInvSys_InventoryItemInstance>();
}

void UInvSys_InventoryFragment_Container::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryFragment_Container, ContainerList)
}

bool UInvSys_InventoryFragment_Container::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bWroteSomething =  Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	// 优化执行流程，只在 ContainerList 被标记为脏时，才会触发循环执行复制！
	// if (Channel->KeyNeedsToReplicate(ContainerList.RepIndex, ContainerList.ArrayReplicationKey))
	{
		// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning,
		// 	TEXT("容器数据被标记为脏，即将同步容器内所有物品至客户端 %d:%d"), ContainerList.RepIndex, ContainerList.ArrayReplicationKey);
		for (FInvSys_ContainerEntry Entry : ContainerList.Entries)
		{
			//if (Channel->KeyNeedsToReplicate(Entry.ReplicationID, Entry.ReplicationKey))
			{
				UInvSys_InventoryItemInstance* Instance = Entry.Instance;
				if (Instance && IsValid(Instance))
				{
					bWroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
				}
			}
			// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("复制 Fast Array 的子对象 = %s]"), *Instance->GetName());
		}
	}
	return bWroteSomething;
}
