// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Components/GridPanel.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridDraggingDisplayWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/Notifications/SNotificationList.h"

void UGridInvSys_ContainerGridWidget::FindContainerGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray,
                                                             FIntPoint Position, FIntPoint ItemSize, const TSet<UGridInvSys_ContainerGridItemWidget*>& Ignores)
{
	OutArray.Empty(ItemSize.X * ItemSize.Y);
	for (int i = 0; i < ItemSize.X; ++i)
	{
		for (int j = 0; j < ItemSize.Y; ++j)
		{
			UGridInvSys_ContainerGridItemWidget* GridItemWidget = GetContainerGridItem(
				FIntPoint(Position.X + i, Position.Y + j));
			// 仅处理 UContainerGridItemWidget 及其子类。
			if (GridItemWidget != nullptr && Ignores.Contains(GridItemWidget) == false)
			{
				OutArray.AddUnique(GridItemWidget);
			}
		}
	}
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridWidget::GetContainerGridItem(FIntPoint Position)
{
	if (Position.X < 0 || Position.Y < 0 ||
		Position.X >= ContainerGridSize.X ||
		Position.Y >= ContainerGridSize.Y)
	{
		return nullptr;
	}

	return Cast<UGridInvSys_ContainerGridItemWidget>(
		ContainerGridPanel->GetChildAt(Position.X * ContainerGridSize.Y + Position.Y));
}

FName UGridInvSys_ContainerGridWidget::GetContainerGridID() const
{
	return ContainerGridID;
}

void UGridInvSys_ContainerGridWidget::SetContainerGridID(FName NewContainerGridID)
{
	ContainerGridID = NewContainerGridID;
}

void UGridInvSys_ContainerGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize * ContainerGridSize.Y);
		SizeBox->SetHeightOverride(ItemDrawSize * ContainerGridSize.X);
	}
}

void UGridInvSys_ContainerGridWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		NativeConstruct();

		// SizeBox设置大小
		NativeConstruct();

		if (SizeBox != GetRootWidget())
		{
			FNotificationInfo Info(FText::FromString(TEXT("SizeBox 推荐设置为 Root Widget。")));
			Info.ExpireDuration = 5.f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
}

void UGridInvSys_ContainerGridWidget::ConstructGridItems(FName NewSlotName)
{
#if WITH_EDITOR
	if (GridItemWidgetClass == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("GridItemWidgetClass is nullptr."))
		return;
	}
#endif
	
	//  清除 ContainerGrid 中所有子项。
	if (ContainerGridPanel->HasAnyChildren() == true)
	{
		ContainerGridPanel->ClearChildren();
		DragDisplayGridPanel->ClearChildren();
	}

	for (int i = 0; i < ContainerGridSize.X * ContainerGridSize.Y; ++i)
	{
		// GridDraggingDisplayWidgetClass
		UGridInvSys_ContainerGridItemWidget* GridItemWidget =
			CreateWidget<UGridInvSys_ContainerGridItemWidget>(this, GridItemWidgetClass);
		UGridInvSys_ContainerGridDraggingDisplayWidget* GridDraggingDisplayWidget =
			CreateWidget<UGridInvSys_ContainerGridDraggingDisplayWidget>(this, GridDraggingDisplayWidgetClass);
		// 将自身传入 Grid Item 中

		GridItemWidget->SetSlotName(NewSlotName);
		GridItemWidget->SetGridID(ContainerGridID);
		
		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		ContainerGridPanel->AddChildToGrid(GridItemWidget, SlotRow,SlotColumn);
		DragDisplayGridPanel->AddChildToUniformGrid(GridDraggingDisplayWidget, SlotRow,SlotColumn);
		
		GridItemWidget->InitContainerGridItem(this, FIntPoint(SlotRow, SlotColumn));
		// OnConstructGridItem(GridItemWidget);
	}

	SlotName = NewSlotName;
	// SlotName = 
}

void UGridInvSys_ContainerGridWidget::AddInventoryItemTo(const FGridInvSys_InventoryItem& InventoryItem)
{
	FIntPoint ToPosition = InventoryItem.ItemPosition.Position; 

	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	if (ToGridItemWidget == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("添加物品失败：ToGridItemWidget is nullptr."));
		return;
	}

	if (ToGridItemWidget->IsOccupied())
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("添加物品失败：ToGridItemWidget 已经被占领."));
		return;
	}

	// 判断位置是否存在足够的空闲空间
	const FIntPoint ItemSize = InventoryItem.ItemPosition.Size;
	if (ToPosition.X + ItemSize.X > ContainerGridSize.X || ToPosition.Y + ItemSize.Y > ContainerGridSize.Y)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("添加物品失败：位置 [%d, %d] 的 [%d, %d] 范围超过了容器最大范围 [%d, %d]"),
			ToPosition.X, ToPosition.Y, ItemSize.X, ItemSize.Y, ContainerGridSize.X, ContainerGridSize.Y);
		return;
	}

	if (HasEnoughFreeSpace(ToPosition, ItemSize))
	{
		ToGridItemWidget->UpdateItemInfo(InventoryItem.BaseItemData.ItemInfo);
	}
}

bool UGridInvSys_ContainerGridWidget::HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize,
	const TArray<UGridInvSys_ContainerGridItemWidget*>& Ignores)
{
	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	GetInventoryItemsByAre(OutArray, IntPoint, ItemSize);
	if (OutArray.IsEmpty() == false)
	{
		// 判断是否为需要忽略的格子。
		for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
		{
			if (Ignores.Contains(GridItemWidget) == false)
			{
				return false;
			}
		}
	}
	if (IntPoint.X >= 0 && IntPoint.X + ItemSize.X <= ContainerGridSize.X &&
		IntPoint.Y >= 0 && IntPoint.Y + ItemSize.Y <= ContainerGridSize.Y)
	{
		return true;
	}
	return false;
}

void UGridInvSys_ContainerGridWidget::GetInventoryItemsByAre(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, FIntPoint Position, FIntPoint Size)
{
	for (int i = 0; i < Size.X; ++i)
	{
		for (int j = 0; j < Size.Y; ++j)
		{
			UGridInvSys_ContainerGridItemWidget* GridItemWidget = GetContainerGridItem(
				FIntPoint(Position.X + i, Position.Y + j));
			// 仅处理 UContainerGridItemWidget 及其子类。
			if (GridItemWidget == nullptr)
			{
				continue;
			}
			// 该网格存在占据者
			if (GridItemWidget->IsOccupied())
			{
				OutArray.AddUnique(GridItemWidget);
			}
		}
	}
	UE_LOG(LogInventorySystem, Log, TEXT("在位置为 [%d, %d] 的 [%d X %d]范围内，找到 %d 个对象。"), Position.X, Position.Y, Size.X, Size.Y, OutArray.Num());
}
