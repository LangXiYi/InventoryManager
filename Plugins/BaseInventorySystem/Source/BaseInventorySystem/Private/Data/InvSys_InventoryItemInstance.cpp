// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemInstance.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryItemInstance::UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer)
{
	UObject* MyOuter = GetOuter();
	if (MyOuter)
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner = InventoryComponent->GetOwner();
			// UE_LOG(LogInventorySystem, Error, TEXT("构建库存物品实例的 Outer[%s]！！！"), *InventoryComponent->GetOwner()->GetName())
		}
	}
}

void UInvSys_InventoryItemInstance::PostDuplicate(bool bDuplicateForPIE)
{
	UObject::PostDuplicate(bDuplicateForPIE);
	UObject* MyOuter = GetOuter();
	if (MyOuter)
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			LastInventoryComponent = InventoryComponent;
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner = InventoryComponent->GetOwner();
			// UE_LOG(LogInventorySystem, Error, TEXT("PostDuplicate 库存物品实例的 Outer[%s]！！！"), *InventoryComponent->GetOwner()->GetName())
		}
	}
}

void UInvSys_InventoryItemInstance::PostRepNotifies()
{
	UObject::PostRepNotifies();

	switch (ReplicateState)
	{
	// case EInvSys_ReplicateState::PreRemove:
	// 	PreReplicatedRemove();
	// 	break;
	case EInvSys_ReplicateState::PostAdd:
		PostReplicatedAdd();
		break;
	case EInvSys_ReplicateState::PostChange:
		PostReplicatedChange();
		break;
	default: ;
	}
}

void UInvSys_InventoryItemInstance::PreReplicatedRemove()
{
	ReplicateState = EInvSys_ReplicateState::None;
	UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Remove"))

	BroadcastRemoveItemInstanceMessage();
}

void UInvSys_InventoryItemInstance::PostReplicatedAdd()
{
	ReplicateState = EInvSys_ReplicateState::None;
	UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Add"))

	BroadcastAddItemInstanceMessage();
}

void UInvSys_InventoryItemInstance::PostReplicatedChange()
{
	ReplicateState = EInvSys_ReplicateState::None;
	UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Change"))
}

bool UInvSys_InventoryItemInstance::HasAuthority() const
{
	check(Owner)
	return Owner->HasAuthority();
}

ENetMode UInvSys_InventoryItemInstance::GetNetMode() const
{
	check(Owner)
	return Owner->GetNetMode();
}

void UInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemUniqueID);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, SlotTag);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, bIsDragging);
}

void UInvSys_InventoryItemInstance::OnRep_IsDragging()
{
	FInvSys_DragItemInstanceMessage DragItemInstanceMessage;
	DragItemInstanceMessage.ItemInstance = this;
	DragItemInstanceMessage.bIsDraggingItem = bIsDragging;

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_DragItem, DragItemInstanceMessage);
}

void UInvSys_InventoryItemInstance::SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef)
{
	ItemDefinition = NewItemDef;
}

void UInvSys_InventoryItemInstance::SetItemUniqueID(FGuid Guid)
{
	ItemUniqueID = Guid;
}

void UInvSys_InventoryItemInstance::SetSlotTag(FGameplayTag Tag)
{
	SlotTag = Tag;
}

void UInvSys_InventoryItemInstance::SetIsDraggingItem(bool NewDragState)
{
	if (bIsDragging != NewDragState)
	{
		bIsDragging = NewDragState;
		
		if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
		{
			OnRep_IsDragging();
		}
	}
}

bool UInvSys_InventoryItemInstance::IsDraggingItemInstance() const
{
	return bIsDragging;
}

void UInvSys_InventoryItemInstance::ReplicatedProperties()
{
	if (bIsReadyReplicatedProperties == false)
	{
		return;
	}
	// 遍历所有需要处理的属性并执行对应的函数！！
	for (FInvSys_ItemInstancePropertyHandle OnRepProperty : RegisterPropertyArrays)
	{
		OnRepProperty.OnRepCallback();
	}
	bIsReadyReplicatedProperties = false;
	RegisterPropertyArrays.Reset();
}

void UInvSys_InventoryItemInstance::RemoveFromInventory()
{
	// SlotTag = FGameplayTag(); //不要移除这些属性
	// InvComp = nullptr;
	Entry_Private = nullptr;
	Container_Private = nullptr;
	// SetSlotTag(FGameplayTag());
}

void UInvSys_InventoryItemInstance::MarkItemInstanceDirty()
{
	// todo::调用容器的 MarkItemDirty 复制属性！！！！
}

void UInvSys_InventoryItemInstance::BroadcastAddItemInstanceMessage()
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetSlotTag();
	ItemChangedMessage.ItemInstance = this;

	if (GetInventoryComponent())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("Broadcast Add { Owner = %s, Tag = %s, this = %s}"),
			*GetInventoryComponent()->GetOwner()->GetName(), *GetSlotTag().ToString(), *this->GetName())
	}



	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

void UInvSys_InventoryItemInstance::BroadcastRemoveItemInstanceMessage()
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetSlotTag();
	ItemChangedMessage.ItemInstance = this;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

const UInvSys_InventoryItemFragment* UInvSys_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
