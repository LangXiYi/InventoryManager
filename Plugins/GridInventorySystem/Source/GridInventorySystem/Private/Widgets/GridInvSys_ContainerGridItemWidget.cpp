// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridItemWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Components/GridSlot.h"
#include "Components/SizeBox.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "Widgets/GridInvSys_DragDropWidget.h"
#include "Widgets/Notifications/SNotificationList.h"

void UGridInvSys_ContainerGridItemWidget::SetContainerGridWidget(UGridInvSys_ContainerGridWidget* InContainerGridWidget)
{
	ContainerGridWidget = InContainerGridWidget;
}

void UGridInvSys_ContainerGridItemWidget::UpdateItemInfo(const FGridInvSys_InventoryItem& NewInventoryItem)
{
	UGridSlot* GridSlot = Cast<UGridSlot>(Slot);
	if (GridSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败 InventoryItemWidget 父级类型不是 GridPanel 。"));
		return;
	}
	UInvSys_InventoryItemInfo* NewItemInfo = NewInventoryItem.BaseItemData.ItemInfo;
	const UGridInvSys_InventoryItemInfo* GridItemInfo = Cast<UGridInvSys_InventoryItemInfo>(NewItemInfo);
	check(GridItemInfo);

	DragDropWidget->UpdateItemInfo(NewItemInfo);

	InventoryItem = NewInventoryItem;
	bIsOccupied = true;
	OriginGridItemWidget = this;

	GridSlot->SetRowSpan(GetItemSize().X);
	GridSlot->SetColumnSpan(GetItemSize().Y);
	GridSlot->SetLayer(100);
	
	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(OutArray, GetPosition(), GetItemSize(), {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->bIsOccupied = true;
		GridItemWidget->InventoryItem = NewInventoryItem;
		GridItemWidget->OriginGridItemWidget = this;
	}
	OnItemInfoChanged(NewItemInfo);
}

void UGridInvSys_ContainerGridItemWidget::RemoveItemInfo()
{
	UGridSlot* GridSlot = Cast<UGridSlot>(Slot);
	if (GridSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败 InventoryItemWidget 父级类型不是 GridPanel 。"));
		return;
	}
	// 先临时缓存，避免清除数据后数据丢失。
	const FIntPoint ItemSize = GetItemSize();

	DragDropWidget->UpdateItemInfo(nullptr);
	InventoryItem = FGridInvSys_InventoryItem();
	bIsOccupied = false;
	OriginGridItemWidget = this;

	GridSlot->SetRowSpan(0);
	GridSlot->SetColumnSpan(0);
	GridSlot->SetLayer(0);

	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->FindContainerGridItems(OutArray, GetPosition(), ItemSize, {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->bIsOccupied = false;
		GridItemWidget->InventoryItem = FGridInvSys_InventoryItem();
		GridItemWidget->OriginGridItemWidget = GridItemWidget;
	}
	OnItemInfoChanged(nullptr);
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridItemWidget::GetContainerGridWidget() const
{
	return ContainerGridWidget;
}

UInvSys_InventoryItemInfo* UGridInvSys_ContainerGridItemWidget::GetItemInfo() const
{
	return InventoryItem.BaseItemData.ItemInfo;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetPosition() const
{
	UGridSlot* GridSlot = Cast<UGridSlot>(Slot);
	check(GridSlot)
	return FIntPoint(GridSlot->GetRow(), GridSlot->GetColumn());
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetOriginPosition() const
{
	return OriginGridItemWidget ? OriginGridItemWidget->GetPosition() : Position;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridItemWidget::GetOriginGridItemWidget() const
{
	return OriginGridItemWidget;
}

bool UGridInvSys_ContainerGridItemWidget::IsOccupied() const
{
	return bIsOccupied;
}

void UGridInvSys_ContainerGridItemWidget::SetSlotName(FName NewSlotName)
{
	SlotName = NewSlotName;
}

void UGridInvSys_ContainerGridItemWidget::SetGridID(FName NewGridID)
{
	GridID = NewGridID;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetItemSize() const
{
	return InventoryItem.ItemPosition.Size;
}

FName UGridInvSys_ContainerGridItemWidget::GetItemUniqueID() const
{
	return InventoryItem.BaseItemData.UniqueID;
}

TArray<UWidget*> UGridInvSys_ContainerGridItemWidget::GetOccupiedGridItems()
{
	TArray<UWidget*> GridItems = {this};
	if (IsOccupied())
	{
		ContainerGridWidget->GetContainerGridItems(GridItems, GetOriginPosition(), GetItemSize());
	}
	return GridItems;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::CalculateRelativePosition(const UGridInvSys_ContainerGridItemWidget* Parent) const
{
	check(Parent);
	if (Parent->GetContainerGridWidget() != this->GetContainerGridWidget())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("无法计算相对位置，因为两个物品格所处容器不同。"))
		return false;
	}
	FIntPoint OutRelativePosition;
	OutRelativePosition.X = GetPosition().X - Parent->GetPosition().X;
	OutRelativePosition.Y = GetPosition().Y - Parent->GetPosition().Y;
	return OutRelativePosition;
}

void UGridInvSys_ContainerGridItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		NativeConstruct();

		if (SizeBox != GetRootWidget())
		{
			FNotificationInfo Info(FText::FromString(TEXT("SizeBox 推荐设置为 Root Widget。")));
			Info.ExpireDuration = 5.f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
}

void UGridInvSys_ContainerGridItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize);
		SizeBox->SetHeightOverride(ItemDrawSize);
	}
	DragDropWidget->SetInventoryComponent(GetInventoryComponent());
	DragDropWidget->SetGridItemWidget(this);
}
