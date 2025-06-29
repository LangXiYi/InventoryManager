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

void UGridInvSys_ContainerGridItemWidget::InitContainerGridItem(UGridInvSys_ContainerGridWidget* InContainerGridWidget, FIntPoint InPosition)
{
	ContainerGridWidget = InContainerGridWidget;
	Position = InPosition;

	/*SlotName = InContainerGridWidget->ContainerName;
	GridID = InContainerGridWidget->ContainerGridID;*/

}

void UGridInvSys_ContainerGridItemWidget::UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo)
{
	if (NewItemInfo == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败，传入的 NewItemInfo 为空。"));
		return;
	}
	
	UE_LOG(LogInventorySystem, Log, TEXT("正在执行 UpdateItemInfo 函数, 在位置 [%d, %d] 处添加新物品."), Position.X, Position.Y);
	
	UGridSlot* GridSlot = Cast<UGridSlot>(Slot);
	if (GridSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败 InventoryItemWidget 父级类型不是 GridPanel 。"));
		return;
	}

	const UGridInvSys_InventoryItemInfo* GridItemInfo = Cast<UGridInvSys_InventoryItemInfo>(NewItemInfo);

	DragDropWidget->UpdateItemInfo(NewItemInfo);

	ItemInfo = NewItemInfo;
	bIsOccupied = true;
	OriginGridItemWidget = this;

	GridSlot->SetRowSpan(GridItemInfo->ItemSize.X);
	GridSlot->SetColumnSpan(GridItemInfo->ItemSize.Y);
	GridSlot->SetLayer(100);
	
	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->FindContainerGridItems(OutArray, Position, GridItemInfo->ItemSize, {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->bIsOccupied = true;
		GridItemWidget->ItemInfo = ItemInfo;
		GridItemWidget->OriginGridItemWidget = this;
	}
	OnItemInfoChanged(NewItemInfo);
}

void UGridInvSys_ContainerGridItemWidget::RemoveItemInfo()
{
	// TODO:: From To 调用 Inventory Component 删除
	UE_LOG(LogInventorySystem, Log, TEXT("正在执行 RemoveItemInfo 函数, 移除 GridItem 位于 [%d, %d] 的物品."), Position.X, Position.Y);
	
	UGridSlot* GridSlot = Cast<UGridSlot>(Slot);
	if (GridSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("更新 ItemInfo 失败 InventoryItemWidget 父级类型不是 GridPanel 。"));
		return;
	}
	
	const UGridInvSys_InventoryItemInfo* GridItemInfo = Cast<UGridInvSys_InventoryItemInfo>(ItemInfo);

	DragDropWidget->UpdateItemInfo(nullptr);

	ItemInfo = nullptr;
	bIsOccupied = false;
	OriginGridItemWidget = nullptr;

	GridSlot->SetRowSpan(0);
	GridSlot->SetColumnSpan(0);
	GridSlot->SetLayer(0);

	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	ContainerGridWidget->FindContainerGridItems(OutArray, Position, GridItemInfo->ItemSize, {this});
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		GridItemWidget->bIsOccupied = false;
		GridItemWidget->ItemInfo = nullptr;
		GridItemWidget->OriginGridItemWidget = nullptr;
	}
	OnItemInfoChanged(nullptr);
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridItemWidget::GetContainerGridWidget() const
{
	return ContainerGridWidget;
}

UObject* UGridInvSys_ContainerGridItemWidget::GetItemInfo() const
{
	return ItemInfo;
}

FIntPoint UGridInvSys_ContainerGridItemWidget::GetPosition() const
{
	return Position;
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

FIntPoint UGridInvSys_ContainerGridItemWidget::CalculateRelativePosition(UGridInvSys_ContainerGridItemWidget* Parent) const
{
	if (Parent == nullptr)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("无法计算相对位置，传入的 Parent 为空。"))
		return FIntPoint(-1, -1);
	}
	
	if (Parent->GetParent() != GetParent())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("无法计算相对位置，因为两个 GridItem 的 Parent 不同。"))
		return FIntPoint(-1, -1);
	}

	return FIntPoint(Position.X - Parent->Position.X, Position.Y - Parent->Position.Y);
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

	int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize);
		SizeBox->SetHeightOverride(ItemDrawSize);
	}
}
