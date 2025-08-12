// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryControllerComponent.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Items/InvSys_PickableItems.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_InventoryHUD.h"


UInvSys_InventoryControllerComponent::UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

UInvSys_InventoryHUD* UInvSys_InventoryControllerComponent::ConstructInventoryHUD()
{
	check(InventoryHUDClass)
	if (InventoryHUDClass)
	{
		if (InventoryHUD == nullptr)
		{
			InventoryHUD = CreateWidget<UInvSys_InventoryHUD>(GetOwner<APlayerController>(), InventoryHUDClass);
			InventoryHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	return InventoryHUD;
}

UInvSys_InventoryHUD* UInvSys_InventoryControllerComponent::GetInventoryHUD() const
{
	check(InventoryHUD);
	return InventoryHUD;
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
		if (bIsSuccessDragItem)
		{
			SetDraggingItemInstance(InItemInstance);
		}
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
		check(InvComp && IsValid(InvComp))
		if (InvComp && IsValid(InvComp))
		{
			InvComp->CancelDragItemInstance(InItemInstance);
			SetDraggingItemInstance(nullptr);
		}
	}
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
	UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag InventoryTag)
{
	if (InvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InventoryComponent is nullptr."), __FUNCTION__)
		return;
	}
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	UInvSys_InventoryComponent* OldInvComp = ItemInstance->GetInventoryComponent();
	FGameplayTag OldInventoryTag = ItemInstance->GetInventoryObjectTag();
	check(InventoryTag.IsValid())
	check(OldInventoryTag.IsValid())
	if (InvComp == OldInvComp && InventoryTag == OldInventoryTag)
	{
		// 装备未发生任何改变
		return;
	}

	bool bHasEquipItemInstance = false;
	bool bIsPayloadItems = false;
	if (OldInvComp != nullptr)
	{
		bHasEquipItemInstance = OldInvComp->HasEquipItemInstance(ItemInstance);
	}
	if (bHasEquipItemInstance)
	{
		/**
		 * 若 ItemInstance 是在其他位置被装备的物品
		 * 且存在容器模块那么就将容器模块内所有物品暂存至 ItemInstance 中
		 */
		auto ContainerFragment = OldInvComp->FindInventoryFragment<UInvSys_InventoryFragment_Container>(OldInventoryTag);
		if (ContainerFragment)
		{
			ContainerFragment->GetAllItemInstance(ItemInstance->PayloadItems);
			bIsPayloadItems = true;
		}
	}

	UInvSys_InventoryItemInstance* NewItemInstance = InvComp->EquipItemInstance(ItemInstance, InventoryTag);
	if (NewItemInstance == nullptr)
	{
		if (bIsPayloadItems == true)
		{
			// 装备物品失败，清空暂存的物品
			ItemInstance->PayloadItems.Empty();
		}
		return;
	}
	if (NewItemInstance != nullptr)
	{
		if (bHasEquipItemInstance)
		{
			/**
			 * 如果这个物品是正在装备中的物品？
			 * 装备成功后需要将物品取消装备！！！
			 */
			OldInvComp->UnEquipItemInstance(OldInventoryTag);
		}
		else
		{
			/**
			 * 如果这个物品是在容器内的一个物品？
			 * 装备成功后需要将物品从它原有的容器中删除！！！
			 */
			OldInvComp->RemoveItemInstance(ItemInstance, OldInventoryTag);
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
			APlayerController* PlayerController = GetOwner<APlayerController>();
			APawn* Pawn = PlayerController->GetPawn();
			FTransform Transform = Pawn->GetTransform();

			// Multi_DropItemInstanceToWorld(InItemInstance, Transform);
			InvComp->DiscardItemInstance(InItemInstance, Transform);
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
	if (InvComp->DragItemInstance(InItemInstance))
	{
		SetDraggingItemInstance(InItemInstance);
	}
}

void UInvSys_InventoryControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 限制仅拥有者同步数据
	DOREPLIFETIME_CONDITION(UInvSys_InventoryControllerComponent, DraggingItemInstance, COND_OwnerOnly);
}

void UInvSys_InventoryControllerComponent::Multi_DropItemInstanceToWorld_Implementation(
	UInvSys_InventoryItemInstance* InItemInstance, const FTransform& Transform)
{
	
}

bool UInvSys_InventoryControllerComponent::HasAuthority() const
{
	ensure(GetOwner());
	return GetOwner() ? GetOwner()->HasAuthority() : false;
}

void UInvSys_InventoryControllerComponent::SetDraggingItemInstance(UInvSys_InventoryItemInstance* NewDragItemInstance)
{
	auto LastDragItemInstance = DraggingItemInstance;
	DraggingItemInstance = NewDragItemInstance;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_DraggingItemInstance(LastDragItemInstance);
	}
}

void UInvSys_InventoryControllerComponent::OnRep_DraggingItemInstance(const TWeakObjectPtr<UInvSys_InventoryItemInstance>& OldDraggingItemInstance)
{
	FInvSys_DragItemInstanceMessage DragItemInstanceMessage;
	DragItemInstanceMessage.ItemInstance = DraggingItemInstance.Get();
	DragItemInstanceMessage.bIsDraggingItem = true;
	if (DragItemInstanceMessage.ItemInstance == nullptr)
	{
		DragItemInstanceMessage.ItemInstance = OldDraggingItemInstance.Get();
		DragItemInstanceMessage.bIsDraggingItem = false;
	}

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_DragItem, DragItemInstanceMessage);
}
