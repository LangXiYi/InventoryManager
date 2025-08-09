// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemInstance.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryItemInstance::UInvSys_InventoryItemInstance()
{
	UObject* MyOuter = GetOuter();
	if (MyOuter)
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner_Private = InventoryComponent->GetOwner();
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
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner_Private = InventoryComponent->GetOwner();
			// UE_LOG(LogInventorySystem, Log, TEXT("PostDuplicate 库存物品实例的 Outer[%s]！！！"), *InventoryComponent->GetOwner()->GetName())
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
		UE_LOG(LogInventorySystem, Log, TEXT("PostRepNotifies:%s"), *GetName())
		PostReplicatedAdd();
		break;
	case EInvSys_ReplicateState::PostChange:
		UE_LOG(LogInventorySystem, Log, TEXT("PostRepNotifies:%s"), *GetName())
		PostReplicatedChange();
		break;
	default: ;
	}
}

void UInvSys_InventoryItemInstance::PreReplicatedRemove()
{
	ReplicateState = EInvSys_ReplicateState::None;
	// UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Remove"))

	BroadcastRemoveItemInstanceMessage();
}

void UInvSys_InventoryItemInstance::PostReplicatedAdd()
{
	ReplicateState = EInvSys_ReplicateState::None;
	// UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Add"))

	BroadcastAddItemInstanceMessage();
}

void UInvSys_InventoryItemInstance::PostReplicatedChange()
{
	ReplicateState = EInvSys_ReplicateState::None;
	// UE_LOG(LogInventorySystem, Error, TEXT("PostRepNotifies -- Change"))
}

bool UInvSys_InventoryItemInstance::HasAuthority() const
{
	check(Owner_Private)
	return Owner_Private->HasAuthority();
}

ENetMode UInvSys_InventoryItemInstance::GetNetMode() const
{
	check(Owner_Private)
	return Owner_Private->GetNetMode();
}

void UInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemUniqueID);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, InventoryObjectTag);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, bIsDragging);
}

void UInvSys_InventoryItemInstance::OnRep_IsDragging()
{
	if (OnDragItemInstance.IsBound())
	{
		OnDragItemInstance.Execute(bIsDragging);
	}
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
	InventoryObjectTag = Tag;
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

TSubclassOf<UInvSys_InventoryItemDefinition> UInvSys_InventoryItemInstance::GetItemDefinition() const
{
	check(ItemDefinition);
	return ItemDefinition;
}

const FGuid& UInvSys_InventoryItemInstance::GetItemUniqueID() const
{
	check(ItemUniqueID.IsValid());
	return ItemUniqueID;
}

const FGameplayTag& UInvSys_InventoryItemInstance::GetInventoryObjectTag() const
{
	check(InventoryObjectTag.IsValid())
	return InventoryObjectTag;
}

void UInvSys_InventoryItemInstance::BroadcastAddItemInstanceMessage()
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = this;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_AddItem, ItemChangedMessage);
}

void UInvSys_InventoryItemInstance::BroadcastRemoveItemInstanceMessage()
{
	FInvSys_InventoryItemChangedMessage ItemChangedMessage;
	ItemChangedMessage.InvComp = GetInventoryComponent();
	ItemChangedMessage.InventoryObjectTag = GetInventoryObjectTag();
	ItemChangedMessage.ItemInstance = this;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Inventory_Message_RemoveItem, ItemChangedMessage);
}

FText UInvSys_InventoryItemInstance::GetItemDisplayName() const
{
	check(ItemDefinition);
	return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->GetItemDisplayName();
}

const UInvSys_InventoryItemFragment* UInvSys_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
