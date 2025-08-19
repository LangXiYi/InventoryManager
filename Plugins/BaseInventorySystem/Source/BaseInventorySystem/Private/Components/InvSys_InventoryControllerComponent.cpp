// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryControllerComponent.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_QuickBar.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Items/InvSys_PickableItems.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_InventoryHUD.h"


UInvSys_InventoryControllerComponent::UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

UInvSys_InventoryHUD* UInvSys_InventoryControllerComponent::ConstructInventoryHUD()
{
	check(InventoryHUDClass)
	if (InventoryHUDClass)
	{
		if (InventoryHUD == nullptr)
		{
			InventoryHUD = CreateWidget<UInvSys_InventoryHUD>(GetOwner<APlayerController>(), InventoryHUDClass);
			// InventoryHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	return InventoryHUD;
}

UInvSys_InventoryHUD* UInvSys_InventoryControllerComponent::GetInventoryHUD() const
{
	check(InventoryHUD);
	return InventoryHUD;
}

void UInvSys_InventoryControllerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Controller = GetOwner<APlayerController>();
	check(Controller)
}

void UInvSys_InventoryControllerComponent::CancelDragItemInstance()
{
	if (DraggingItemInstance.Get())
	{
		if (UInvSys_InventoryComponent* InvComp = DraggingItemInstance->GetInventoryComponent())
		{
			InvComp->CancelDragItemInstance(DraggingItemInstance.Get());
		}
	}
	SetDraggingItemInstance(nullptr);
}

void UInvSys_InventoryControllerComponent::Server_SplitItemInstance_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance, int32 SplitSize)
{
	// InvComp->SplitItemInstance<UInvSys_InventoryItemInstance>(ItemInstance, SplitSize, InventoryTag, Args...);
	checkNoEntry();
}

void UInvSys_InventoryControllerComponent::Server_RemoveItemInstance_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance)
	{
		UInvSys_InventoryComponent* InvComp = ItemInstance->GetInventoryComponent();
		check(InvComp)
		if (InvComp)
		{
			InvComp->RemoveItemInstance(ItemInstance);
		}
	}
}

void UInvSys_InventoryControllerComponent::Server_UnEquipItemInstance_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	UInvSys_InventoryComponent* InvComp = ItemInstance->GetInventoryComponent();
	if (InvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InvComp is nullptr."), __FUNCTION__)
		return;
	}
	
	if (InvComp->IsEquippedItemInstance(ItemInstance))
	{
		InvComp->UnEquipItemInstance(ItemInstance);
	}
}

void UInvSys_InventoryControllerComponent::Server_PickupItemInstance_Implementation(UInvSys_InventoryComponent* InvComp,
                                                                                    AInvSys_PickableItems* PickableItems, bool bIsAutoEquip)
{
	if (PickableItems)
	{
		// 返回true时，所有物品都被拾取
		if (PickableItems->PickupItem(InvComp, bIsAutoEquip))
		{
			PickableItems->Destroy();
		}
	}
}

void UInvSys_InventoryControllerComponent::Server_UnEquipItemInstanceByTag_Implementation(
	UInvSys_InventoryComponent* InvComp, FGameplayTag InventoryTag)
{
	if (InvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InvComp is nullptr."), __FUNCTION__)
		return;
	}
	if (InvComp->IsValidInventoryTag(InventoryTag) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InventoryTag is not valid."), __FUNCTION__)
		return;
	}
	if (InvComp->HasEquippedItemInstance(InventoryTag))
	{
		InvComp->UnEquipItemInstanceByTag(InventoryTag);
	}
}

void UInvSys_InventoryControllerComponent::Server_CancelDragItemInstance_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(DraggingItemInstance == nullptr || DraggingItemInstance == InItemInstance)
	CancelDragItemInstance();
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
	if (ItemInstance != DraggingItemInstance)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance != DraggingItemInstance."), __FUNCTION__)
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
		bHasEquipItemInstance = OldInvComp->IsEquippedItemInstance(ItemInstance);
	}
	if (bHasEquipItemInstance)
	{
		/**
		 * 若 ItemInstance 是在其他位置被装备的物品
		 * 且存在容器模块那么就将容器模块内所有物品暂存至 ItemInstance 中
		 */
		auto ContainerFragment = OldInvComp->FindInventoryModule<UInvSys_InventoryModule_Container>(OldInventoryTag);
		if (ContainerFragment)
		{
			ContainerFragment->GetAllItemInstance(ItemInstance->PayloadItems);
			ContainerFragment->RemoveAllItemInstance();
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
			 * 如果这个物品是正在装备中的物品？装备成功后需要将物品取消装备！！！
			 */
			OldInvComp->UnEquipItemInstanceByTag(OldInventoryTag);
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
#if WITH_EDITOR && 1 // Debug_Print
	auto ContainerFragment = OldInvComp->FindInventoryModule<UInvSys_InventoryModule_Container>(OldInventoryTag);
	if (ContainerFragment)
	{
		TArray<UInvSys_InventoryItemInstance*> AllItemInstances;
		ContainerFragment->GetAllItemInstance(AllItemInstances);
		UE_LOG(LogInventorySystem, Log, TEXT("Old Container:%s --- Num = %d"), *OldInventoryTag.ToString(), AllItemInstances.Num())
		for (UInvSys_InventoryItemInstance* OldItem : AllItemInstances)
		{
			UE_LOG(LogInventorySystem, Log, TEXT("Old Item:%s[%d]"), *OldItem->GetItemDisplayName().ToString(), OldItem->GetItemStackCount())
		}
	}
#endif
}

void UInvSys_InventoryControllerComponent::Server_DropItemInstanceToWorld_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		UInvSys_InventoryComponent* InvComp = InItemInstance->GetInventoryComponent();
		if (InvComp)
		{
			check(Controller)
			if (APawn* Pawn = Controller->GetPawn())
			{
				FTransform Transform = Pawn->GetTransform();
				InvComp->DiscardItemInstance(InItemInstance, Transform);
			}
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

void UInvSys_InventoryControllerComponent::Server_SuperposeItemInstance_Implementation(
	UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance)
{
	if (FromItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	// todo:: if BaseItemFragment->bAllowSuperpose == true
	ToItemInstance->SuperposeItemInstance(FromItemInstance);
	if (FromItemInstance->GetItemStackCount() <= 0)
	{
		FromItemInstance->RemoveAndDestroyFromInventory();
		FromItemInstance->ConditionalBeginDestroy();
	}
}

void UInvSys_InventoryControllerComponent::Server_SwapItemInstance_Implementation(
	UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance)
{
	checkNoEntry();
}

void UInvSys_InventoryControllerComponent::Server_UpdateQuickBarItemReference_Implementation(
	UInvSys_InventoryComponent* InvComp, FGameplayTag InventoryTag, UInvSys_InventoryItemInstance* ItemInstance,
	int32 Index)
{
	if (InvComp)
	{
		auto QuickBarModule = InvComp->FindInventoryModule<UInvSys_InventoryModule_QuickBar>(InventoryTag);
		if (QuickBarModule)
		{
			QuickBarModule->UpdateQuickBarItemReference(ItemInstance, Index);
		}
	}
}

void UInvSys_InventoryControllerComponent::Server_UseItemInstance_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance)
	{
		ItemInstance->UseItemInstance();
	}
}

bool UInvSys_InventoryControllerComponent::HasAuthority() const
{
	ensure(GetOwner());
	return GetOwner() ? GetOwner()->HasAuthority() : false;
}

APlayerController* UInvSys_InventoryControllerComponent::GetPlayerController() const
{
	return Controller;
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
