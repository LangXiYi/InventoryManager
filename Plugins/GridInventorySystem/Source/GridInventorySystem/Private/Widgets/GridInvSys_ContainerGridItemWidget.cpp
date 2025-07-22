// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridItemWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridSlot.h"
#include "Components/SizeBox.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "Widgets/Notifications/SNotificationList.h"

void UGridInvSys_ContainerGridItemWidget::OnConstructGridItem(UGridInvSys_ContainerGridWidget* InContainerGrid,
	FIntPoint InPosition)
{
	ContainerGridWidget = InContainerGrid;
	Position = InPosition;
	OriginGridItemWidget = this;
	GridItemSize = FIntPoint(1, 1);
	bIsOccupied = false;
	InventoryComponent = InContainerGrid->GetInventoryComponent();
}

void UGridInvSys_ContainerGridItemWidget::UpdateItemInfo(const FGridInvSys_InventoryItem& NewInventoryItem)
{
	checkNoEntry()
}

void UGridInvSys_ContainerGridItemWidget::RemoveItemInfo()
{
	checkNoEntry()
}

void UGridInvSys_ContainerGridItemWidget::AddItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	/*UCanvasPanelSlot* GridSlot = Cast<UCanvasPanelSlot>(Slot);
	if (GridSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败 InventoryItemWidget 父级类型不是 GridPanel 。"));
		return;
	}*/
	if (NewItemInstance == nullptr || NewItemInstance->GetItemDefinition() == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("NewItemInstance 为空。"));
		return;
	}

	bIsOccupied = true;
	OriginGridItemWidget = this;
	ItemInstance = NewItemInstance;
	GridItemSize = CalculateGridItemSize(NewItemInstance);

	/*GridSlot->SetRowSpan(GridItemSize.X);
	GridSlot->SetColumnSpan(GridItemSize.Y);
	GridSlot->SetLayer(100);*/

	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(OutArray, GetPosition(), GridItemSize, {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->RemoveItemInstance();
		GridItemWidget->bIsOccupied = true;
		GridItemWidget->ItemInstance = NewItemInstance;
		GridItemWidget->OriginGridItemWidget = this;
	}
	OnAddItemInstance(NewItemInstance);
}

void UGridInvSys_ContainerGridItemWidget::RemoveItemInstance()
{
	// 先移除下级网格的信息
	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->FindContainerGridItems(OutArray, GetPosition(), GridItemSize, {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->RemoveItemInstance();
	}

	ItemInstance = nullptr;
	GridItemSize = FIntPoint(1, 1);
	bIsOccupied = false;
	OriginGridItemWidget = this;
	OnRemoveItemInstance();
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridItemWidget::GetContainerGridWidget() const
{
	return ContainerGridWidget;
}

UInvSys_InventoryItemInfo* UGridInvSys_ContainerGridItemWidget::GetItemInfo() const
{
	return nullptr;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetPosition() const
{
	return Position;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetOriginPosition() const
{
	return OriginGridItemWidget ? OriginGridItemWidget->GetPosition() : GetPosition();
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridItemWidget::GetOriginGridItemWidget() const
{
	return OriginGridItemWidget;
}

EGridInvSys_ItemDirection UGridInvSys_ContainerGridItemWidget::GetItemDirection() const
{
	return EGridInvSys_ItemDirection::Horizontal;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::CalculateGridItemSize(UInvSys_InventoryItemInstance* InItemInstance) const
{
	FIntPoint Result = FIntPoint(1, 1);
	if (InItemInstance && InItemInstance->IsA(UGridInvSys_InventoryItemInstance::StaticClass()))
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		check(GridItemInstance);
		FGridInvSys_ItemPosition ItemPosition = GridItemInstance->GetItemPosition();
		if (auto SizeFragment = GridItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
		{
			Result = SizeFragment->ItemSize;
		}
		switch (ItemPosition.Direction)
		{
		case EGridInvSys_ItemDirection::Horizontal:
			return Result;
		case EGridInvSys_ItemDirection::Vertical:
			return FIntPoint(Result.Y, Result.X);
		}
	}
	return Result;
}

FName UGridInvSys_ContainerGridItemWidget::GetItemUniqueID() const
{
	return NAME_None;
}

FName UGridInvSys_ContainerGridItemWidget::GetSlotName() const
{
	return NAME_None;
}

TArray<UWidget*> UGridInvSys_ContainerGridItemWidget::GetOccupiedGridItems()
{
	TArray<UWidget*> GridItems = {this};
	/*if (IsOccupied())
	{
		ContainerGridWidget->GetContainerGridItems(GridItems, GetOriginPosition(), GetItemSize());
	}*/
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

int32 UGridInvSys_ContainerGridItemWidget::GetGridID() const
{
	UGridInvSys_ContainerGridWidget* TempWidget = GetContainerGridWidget();
	check(TempWidget)
	return TempWidget->GetContainerGridID();
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
}