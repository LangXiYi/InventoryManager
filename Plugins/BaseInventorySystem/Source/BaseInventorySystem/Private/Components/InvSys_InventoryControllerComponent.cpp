// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryControllerComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Engine/ActorChannel.h"
#include "Misc/LowLevelTestAdapter.h"
#include "Net/UnrealNetwork.h"


UInvSys_InventoryControllerComponent::UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInvSys_InventoryControllerComponent::Server_DragAndRemoveItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InvComp)
	check(InItemInstance)
	bool bIsSuccessDragItem = false;
	if (InvComp && InItemInstance)
	{
		bIsSuccessDragItem = InvComp->DragAndRemoveItemInstance(InItemInstance);
		DraggingItemInstance = bIsSuccessDragItem ? InItemInstance : nullptr;
	}
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && bIsSuccessDragItem == false, LogInventorySystem, Warning,
		TEXT("尝试拽起物品实例失败！！"))
}

void UInvSys_InventoryControllerComponent::Server_CancelDragItemInstance_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	if (InItemInstance)
	{
		UInvSys_InventoryComponent* InvComp = InItemInstance->GetInventoryComponent();
		check(InvComp)
		if (InvComp)
		{
			InvComp->CancelDragItemInstance(InItemInstance);
			DraggingItemInstance = nullptr;
		}
	}
}

void UInvSys_InventoryControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInvSys_InventoryControllerComponent, DraggingItemInstance, COND_OwnerOnly);
}

void UInvSys_InventoryControllerComponent::OnRep_DraggingItemInstance()
{
	FInvSys_DragItemInstanceMessage DragItemInstanceMessage;
	DragItemInstanceMessage.ItemInstance = DraggingItemInstance.Get();
	DragItemInstanceMessage.bIsDraggingItem = DraggingItemInstance.IsValid();

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_DragItem, DragItemInstanceMessage);
}

bool UInvSys_InventoryControllerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                               FReplicationFlags* RepFlags)
{
	// 持续同步被拽起的物品
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	// if (DraggingItemInstance.IsValid())
	// {
	// 	// UActorChannel::SetCurrentSubObjectOwner(DraggingItemInstance->GetInventoryComponent());
	// 	if (DraggingItemInstance->MyInstances.Num() > 0)
	// 	{
	// 		for (UInvSys_InventoryItemInstance* ItemInstance : DraggingItemInstance->MyInstances)
	// 		{
	// 			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, * RepFlags);
	// 		}
	// 	}
	// 	bWroteSomething |= Channel->ReplicateSubobject(DraggingItemInstance.Get(), *Bunch, * RepFlags);
	// }
	return bWroteSomething;
}

void UInvSys_InventoryControllerComponent::Server_EquipItemDefinition_Implementation(
	UInvSys_InventoryComponent* InvComp, TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag)
{
	check(InvComp);
	if (InvComp == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("传入的库存组件不存在。"))
		return;
	}
	if (ItemDef == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("传入的物品定义不存在。"))
		return;
	}
	auto DefaultItemDefinition = GetDefault<UInvSys_InventoryItemDefinition>(ItemDef);
	auto EquipItemFragment = DefaultItemDefinition->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
	if (EquipItemFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品[%s]未添加装备片段。"),
			*DefaultItemDefinition->GetItemDisplayName().ToString())
		return;
	}
	if (EquipItemFragment->SupportEquipSlot.HasTagExact(SlotTag) == false)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品[%s]不支持装备到目标槽位[%s]"),
			*DefaultItemDefinition->GetItemDisplayName().ToString(), *SlotTag.ToString())
		return;
	} 
	InvComp->EquipItemDefinition(ItemDef, SlotTag);
}

void UInvSys_InventoryControllerComponent::Server_EquipItemInstance_Implementation(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag)
{
	check(InvComp);
	if (InvComp == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("传入的库存组件不存在。"))
		return;
	}
	if (InItemInstance == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("传入的物品实例不存在。"))
		return;
	}
	auto EquipItemFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
	if (EquipItemFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品[%s]未添加装备片段。"),
			*InItemInstance->GetItemDisplayName().ToString())
		return;
	}
	if (EquipItemFragment->SupportEquipSlot.HasTagExact(SlotTag) == false)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品[%s]不支持装备到目标槽位[%s]"),
			*InItemInstance->GetItemDisplayName().ToString(), *SlotTag.ToString())
		return;
	} 
	InvComp->EquipItemInstance(InItemInstance, SlotTag);
}

void UInvSys_InventoryControllerComponent::Server_RestoreItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InvComp)
	if (InvComp)
	{
		bool bIsSuccess = InvComp->RestoreItemInstance(InItemInstance);
		if (bIsSuccess == false)
		{
			check(false);
			//todo::丢弃至世界？
		}
	}
}

void UInvSys_InventoryControllerComponent::Server_DropItemInstanceToWorld_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		UInvSys_InventoryComponent* InvComp = InItemInstance->GetInventoryComponent();
		if (InvComp)
		{
			InvComp->DropItemInstanceToWorld(InItemInstance);
		}
	}
}

void UInvSys_InventoryControllerComponent::Server_DragItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Failed, Inventory Component is nullptr."), __FUNCTION__)
		return;
	}
	if (InItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Failed, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	bool bDragItemInstance = InvComp->DragItemInstance(InItemInstance);
	DraggingItemInstance = bDragItemInstance ? InItemInstance : nullptr;
}

