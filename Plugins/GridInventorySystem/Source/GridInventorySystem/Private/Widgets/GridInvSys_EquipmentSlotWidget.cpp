// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipmentSlotWidget.h"

#include "BaseInventorySystem.h"
#include "InvSys_CommonType.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"
#include "Data/GridInvSys_ItemFragment_ItemType.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/GridInvSys_DragItemWidget.h"

void UGridInvSys_EquipmentSlotWidget::RefreshInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	Super::RefreshInventoryWidget(NewInventoryObject);
	check(InventoryObject)
	auto EquipFragment = InventoryObject->FindInventoryFragment<UInvSys_InventoryFragment_Equipment>();
	if (EquipFragment)
	{
		UInvSys_InventoryItemInstance* TargetItemInstance = EquipFragment->GetEquipItemInstance();
		if (ItemInstance != TargetItemInstance)
		{
			UnEquipItemInstance();
			if (EquipFragment && EquipFragment->HasEquipmentItems())
			{
				EquipItemInstance(TargetItemInstance);
			}
		}
	}
}

void UGridInvSys_EquipmentSlotWidget::EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	check(NewItemInstance)
	if (NewItemInstance)
	{
		ItemInstance = NewItemInstance;
		OnEquipInventoryItem(NewItemInstance);
	}
}

void UGridInvSys_EquipmentSlotWidget::UnEquipItemInstance()
{
	if (ItemInstance != nullptr)
	{
		ItemInstance = nullptr;
		OnUnEquipInventoryItem();
	}
}

void UGridInvSys_EquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	auto WarpEquipItemFunc = [this](FGameplayTag Tag, const FInvSys_EquipItemInstanceMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			EquipItemInstance(Message.ItemInstance);
		}
	};

	auto WarpUnEquipItemFunc = [this](FGameplayTag Tag, const FInvSys_EquipItemInstanceMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			UnEquipItemInstance();
		}
	};

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	OnEquipItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_EquipItemInstanceMessage>(
		Inventory_Message_EquipItem, MoveTemp(WarpEquipItemFunc));

	OnUnEquipItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_EquipItemInstanceMessage>(
		Inventory_Message_UnEquipItem, MoveTemp(WarpUnEquipItemFunc));
}

void UGridInvSys_EquipmentSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();
	OnEquipItemInstanceHandle.Unregister();
	OnUnEquipItemInstanceHandle.Unregister();
}

bool UGridInvSys_EquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                                   UDragDropOperation* InOperation)
{
	UInvSys_InventoryControllerComponent* ICC =
		UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent<UInvSys_InventoryControllerComponent>(GetWorld());
	check(ICC)
	if (ICC == nullptr)
	{
		return false;
	}
	UInvSys_InventoryItemInstance* PayLoadItemInstance = Cast<UInvSys_InventoryItemInstance>(InOperation->Payload);
	check(PayLoadItemInstance)
	if (PayLoadItemInstance)
	{
		// 这里是客户端的本地检查，在服务器还有一重检查，确保目标物品不会装备到错误的位置。
		auto EquipItemFragment = PayLoadItemInstance->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
		if (EquipItemFragment)
		{
			// 判断目标物品是否支持在该槽位装备
			if (EquipItemFragment->SupportEquipSlot.HasTagExact(SlotTag) && EquipItemFragment->bIsAllowPlayerChange == true)
			{
				ICC->Server_EquipItemInstance(InventoryComponent.Get(), PayLoadItemInstance, SlotTag);
				return true;
			}
		}
		// // 还原目标物品
		// UInvSys_InventoryComponent* From_InvComp = PayLoadItemInstance->GetInventoryComponent();
		// ICC->Server_RestoreItemInstance(From_InvComp, PayLoadItemInstance); 
	}
	return false;
}
