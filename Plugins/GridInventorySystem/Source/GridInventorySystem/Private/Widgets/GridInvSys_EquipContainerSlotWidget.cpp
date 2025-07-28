// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipContainerSlotWidget.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "GridInvSys_CommonType.h"
#include "NativeGameplayTags.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "BaseInventorySystem.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"

void UGridInvSys_EquipContainerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto WarpAddItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			check(ContainerLayout)
			if (ContainerLayout)
			{
				ContainerLayout->AddItemInstance(Message.ItemInstance);
			}
		}
	};

	auto WarpRemoveItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			check(ContainerLayout)
			if (ContainerLayout)
			{
				ContainerLayout->RemoveItemInstance(Message.ItemInstance);
			}
		}
	};

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	OnAddItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_AddItem, MoveTemp(WarpAddItemFunc));

	OnRemoveItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
		Inventory_Message_RemoveItem, MoveTemp(WarpRemoveItemFunc));
}

void UGridInvSys_EquipContainerSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnAddItemInstanceHandle.Unregister();
	OnRemoveItemInstanceHandle.Unregister();
}

void UGridInvSys_EquipContainerSlotWidget::RefreshWidget()
{
	Super::RefreshWidget();
	// 为什么不在这里调用 ContainerLayout 的 RefreshWidget？
	// 因为在 EquipItemInstance 在 RefreshWidget中已经被调用了，为了避免重复调用，故只需要在 EquipItemInstance 中调用即可
	// ContainerLayout->RefreshWidget();
}

void UGridInvSys_EquipContainerSlotWidget::EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	Super::EquipItemInstance(NewItemInstance);
	if (NewItemInstance)
	{
		auto ContainerLayoutFragment = NewItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		check(ContainerLayoutFragment)
		if (ContainerLayoutFragment)
		{
			ContainerLayout = CreateWidget<UGridInvSys_ContainerGridLayoutWidget>(this, ContainerLayoutFragment->ContainerLayout);
			check(ContainerLayout)
			ContainerLayout->SetInventoryObject(GetInventoryObject());
			ContainerLayout->RefreshWidget();
			NS_ContainerGridLayout->AddChild(ContainerLayout);
		}
	}
}

void UGridInvSys_EquipContainerSlotWidget::UnEquipItemInstance()
{
	Super::UnEquipItemInstance();
	if (ContainerLayout)
	{
		ContainerLayout->RemoveFromParent();
		ContainerLayout->ConditionalBeginDestroy();
		ContainerLayout = nullptr;
	}
}
