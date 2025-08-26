// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemInstance.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryItemInstance::UInvSys_InventoryItemInstance()
{
	InventoryComponent = GetTypedOuter<UInvSys_InventoryComponent>();
	if (InventoryComponent)
	{
		Owner_Private = InventoryComponent->GetOwner();
	}
}

void UInvSys_InventoryItemInstance::UseItemInstance()
{
	if (IsUsableItemInstance())
	{
		NativeOnUseItemInstance();
	}
}

void UInvSys_InventoryItemInstance::NativeOnUseItemInstance()
{
	OnUseItemInstance();
}

void UInvSys_InventoryItemInstance::PostDuplicate(bool bDuplicateForPIE)
{
	UObject::PostDuplicate(bDuplicateForPIE);
	InventoryComponent = GetTypedOuter<UInvSys_InventoryComponent>();
	if (InventoryComponent)
	{
		Owner_Private = InventoryComponent->GetOwner();
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

void UInvSys_InventoryItemInstance::RemoveAndDestroyFromInventory()
{
	if (InventoryComponent)
	{
		auto EquipmentModule = InventoryComponent->FindInventoryModule<UInvSys_InventoryModule_Equipment>(InventoryTag);
		if (EquipmentModule)
		{
			if (EquipmentModule->GetEquipItemInstance() == this)
			{
				EquipmentModule->UnEquipItemInstance();
				return;
			}
		}
		
		if (ContainerModule)
		{
			if (ContainerModule->ContainsItem(this))
			{
				ContainerModule->RemoveItemInstance(this);
			}
		}
		ConditionalBeginDestroy();
	}
}

UGameplayTasksComponent* UInvSys_InventoryItemInstance::GetGameplayTasksComponent(const UGameplayTask& Task) const
{
	return IGameplayTaskOwnerInterface::GetGameplayTasksComponent(Task);
}

AActor* UInvSys_InventoryItemInstance::GetGameplayTaskOwner(const UGameplayTask* Task) const
{
	return IGameplayTaskOwnerInterface::GetGameplayTaskOwner(Task);
}

AActor* UInvSys_InventoryItemInstance::GetGameplayTaskAvatar(const UGameplayTask* Task) const
{
	return IGameplayTaskOwnerInterface::GetGameplayTaskAvatar(Task);
}

void UInvSys_InventoryItemInstance::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskInitialized(Task);
}

void UInvSys_InventoryItemInstance::OnGameplayTaskActivated(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskActivated(Task);
}

void UInvSys_InventoryItemInstance::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskDeactivated(Task);
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

bool UInvSys_InventoryItemInstance::HasEquipment()
{
	if (InventoryComponent)
	{
		return InventoryComponent->IsEquippedItemInstance(this);
	}
	return false;
}

void UInvSys_InventoryItemInstance::SetItemStackCount(int32 NewStackCount)
{
	StackCount = NewStackCount;
	MarkItemInstanceDirty();
}

void UInvSys_InventoryItemInstance::SuperposeItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	if (ItemInstance == this)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance == this."), __FUNCTION__)
		return;
	}
	if (GetItemDefinition() != ItemInstance->GetItemDefinition())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, This ItemDefinition != FromItemInstance->ItemDefinition."), __FUNCTION__)
		return;
	}
	int32 MaxStackCount = GetItemMaxStackCount();
	int32 FromItemStackCount = ItemInstance->GetItemStackCount();
	// 剩余可堆叠数量与 FromInstanceStackCount 之间取最小值
	int32 DeltaStackCount = FMath::Min(MaxStackCount - StackCount, FromItemStackCount);

	/**
	 * 在修改堆叠数量前先处理其他属性
	 * 比如物品新鲜度，需要在堆叠时将新鲜度按平均值重新计算
	 *		A: FreshTime = 10  StackCount = 1
	 *		B: FreshTime 5 StackCount = 4
	 *		将物品B全部加入物品A中，最终物品A的新鲜度为：(10 + 5 * 4) / 5 = 6
	 */
	PreUpdateItemStackCount(ItemInstance, DeltaStackCount);

	// 更新当前物品实例的数量
	SetItemStackCount(StackCount + DeltaStackCount);
	ItemInstance->SetItemStackCount(FromItemStackCount - DeltaStackCount);
}

void UInvSys_InventoryItemInstance::PreUpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance,
	int32 DeltaStackCount)
{
	BP_PreUpdateItemStackCount(ItemInstance, DeltaStackCount);
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

void UInvSys_InventoryItemInstance::MarkItemInstanceDirty()
{
	if (ContainerModule)
	{
		ContainerModule->MarkItemInstanceDirty(this);
	}
}

void UInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryItemInstance, StackCount);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, bIsDragging);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, InventoryTag);
}

void UInvSys_InventoryItemInstance::OnRep_InventoryTag()
{
	if (InventoryComponent)
	{
		ContainerModule = InventoryComponent->FindInventoryModule<UInvSys_InventoryModule_Container>(InventoryTag);
	}
}

void UInvSys_InventoryItemInstance::OnRep_IsDragging()
{
	if (OnDragItemInstance.IsBound())
	{
		OnDragItemInstance.Execute(bIsDragging);
	}
}

void UInvSys_InventoryItemInstance::SetInventoryTag(FGameplayTag Tag)
{
	InventoryTag = Tag;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_InventoryTag();
	}
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
		MarkItemInstanceDirty();
	}
}

int32 UInvSys_InventoryItemInstance::GetItemRemainStackCount() const
{
	auto BaseItemFragment = FindFragmentByClass<UInvSys_ItemFragment_BaseItem>();
	return BaseItemFragment ? BaseItemFragment->MaxStackCount - StackCount : 0;
}

int32 UInvSys_InventoryItemInstance::GetItemMaxStackCount() const
{
	auto BaseItemFragment = FindFragmentByClass<UInvSys_ItemFragment_BaseItem>();
	return BaseItemFragment ? BaseItemFragment->MaxStackCount : 1;
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
