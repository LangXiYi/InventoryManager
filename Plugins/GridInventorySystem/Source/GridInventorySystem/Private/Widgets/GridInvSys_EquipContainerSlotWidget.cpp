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

UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_AddItem, "Inventory.Message.AddItem");
UE_DEFINE_GAMEPLAY_TAG(Inventory_Message_RemoveItem, "Inventory.Message.RemoveItem");

void UGridInvSys_EquipContainerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto WarpAddItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		this->OnAddItemInstance(Tag, Message);
	};

	auto WarpRemoveItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		this->OnRemoveItemInstance(Tag, Message);
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

void UGridInvSys_EquipContainerSlotWidget::OnAddItemInstance(FGameplayTag Tag,
                                                             const FInvSys_InventoryItemChangedMessage& Message)
{
	AddItemInstance(Message.ItemInstance);
}

void UGridInvSys_EquipContainerSlotWidget::OnRemoveItemInstance(FGameplayTag Tag,
	const FInvSys_InventoryItemChangedMessage& Message)
{
	RemoveItemInstance(Message.ItemInstance);
}

/*
UGridInvSys_ContainerGridItemWidget* UGridInvSys_EquipContainerSlotWidget::FindGridItemWidget(
	const FGridInvSys_ItemPosition& ItemPosition) const
{
	if (GetSlotTag() != ItemPosition.EquipSlotTag)
	{
		return nullptr;
	}
	if (ContainerLayoutWidget && ContainerLayoutWidget->IsA(UGridInvSys_ContainerGridLayoutWidget::StaticClass()))
	{
		UGridInvSys_ContainerGridLayoutWidget* LayoutWidget = Cast<UGridInvSys_ContainerGridLayoutWidget>(ContainerLayoutWidget);
		check(LayoutWidget);
		return LayoutWidget->FindGridItemWidget(ItemPosition);
	}
	return nullptr;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_EquipContainerSlotWidget::FindGridItemWidget(
	const UInvSys_InventoryItemInstance* NewItemInstance) const
{
	if (NewItemInstance && NewItemInstance->IsA(UGridInvSys_InventoryItemInstance::StaticClass()))
	{
		UGridInvSys_InventoryItemInstance* TempItemInstance = (UGridInvSys_InventoryItemInstance*)NewItemInstance;
		check(TempItemInstance);
		return FindGridItemWidget(TempItemInstance->GetItemPosition());
	}
	return nullptr;
}
*/

void UGridInvSys_EquipContainerSlotWidget::AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr)
	{
		return;
	}
	if (InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		AddItemInstanceTo(GridItemInstance, GridItemInstance->GetItemPosition());
	}
}

void UGridInvSys_EquipContainerSlotWidget::AddItemInstanceTo(
	UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition)
{
	if (InItemInstance == nullptr)
	{
		return;
	}
	if (InItemInstance->GetInventoryComponent() != InventoryComponent)
	{
		// 添加的物品必须是在同一组件下的。
		return;
	}
	if (SlotTag != InPosition.EquipSlotTag)
	{
		return;
	}
	UGridInvSys_ContainerGridLayoutWidget* GridContainerLayout = GetContainerLayoutWidget<UGridInvSys_ContainerGridLayoutWidget>();
	if (GridContainerLayout)
	{
		UGridInvSys_ContainerGridItemWidget* GridItemWidget = GridContainerLayout->FindGridItemWidget(InPosition);
		if (GridItemWidget)
		{
			if (GridItemWidget->IsOccupied() == false)
			{
				UE_LOG(LogInventorySystem, Log, TEXT("正在为新位置添加物品==>[%s]"), *InPosition.ToString())
				GridItemWidget->AddItemInstance(InItemInstance);
			}
			else
			{
				UInvSys_InventoryControllerComponent* PlayerInvComp = UInvSys_InventorySystemLibrary::FindInvControllerComponent(GetWorld());
				if (PlayerInvComp)
				{
					// PlayerInvComp->Server_RestoreItemInstance(InItemInstance);
					PlayerInvComp->Server_DropItemInstanceToWorld(InItemInstance);
				}
			}
		}
	}
}

void UGridInvSys_EquipContainerSlotWidget::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr)
	{
		return;
	}
	if (InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		RemoveItemInstanceFor(GridItemInstance, GridItemInstance->GetItemPosition());
	}
	else
	{
		UGridInvSys_ContainerGridLayoutWidget* GridContainerLayout = GetContainerLayoutWidget<UGridInvSys_ContainerGridLayoutWidget>();
		if (GridContainerLayout)
		{
			// 寻找内部所有的网格，判断物品在容器内是否存在
			TArray<UGridInvSys_ContainerGridWidget*> ContainerGridWidgets = GridContainerLayout->GetContainerGridWidgets();
			for (UGridInvSys_ContainerGridWidget* ContainerGridWidget : ContainerGridWidgets)
			{
				TArray<UGridInvSys_ContainerGridItemWidget*> GridItemWidgets = ContainerGridWidget->GetAllContainerGridItems();
				for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : GridItemWidgets)
				{
					if (GridItemWidget && GridItemWidget->GetItemInstance() == InItemInstance)
					{
						GridItemWidget->RemoveItemInstance();
						return;
					}
				}
			}
			UE_LOG(LogInventorySystem, Warning, TEXT("移除物品失败，指定的物品实例在容器内部不存在！"))
		}
	}
}

void UGridInvSys_EquipContainerSlotWidget::RemoveItemInstanceFor(UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPosition)
{
	if (InItemInstance == nullptr)
	{
		return;
	}
	UGridInvSys_ContainerGridLayoutWidget* GridContainerLayout = GetContainerLayoutWidget<UGridInvSys_ContainerGridLayoutWidget>();
	if (GridContainerLayout)
	{
		UGridInvSys_ContainerGridItemWidget* GridItemWidget = GridContainerLayout->FindGridItemWidget(InPosition);
		if (GridItemWidget && GridItemWidget->GetItemInstance() == InItemInstance)
		{
			GridItemWidget->RemoveItemInstance();
		}
	}
}