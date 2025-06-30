// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/GridPanel.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridDropWidget.h"
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
			UGridInvSys_ContainerGridItemWidget* GridItemWidget = GetContainerGridItem(FIntPoint(Position.X + i, Position.Y + j));
			// 仅处理 UContainerGridItemWidget 及其子类。
			if (GridItemWidget != nullptr && Ignores.Contains(GridItemWidget) == false)
			{
				OutArray.AddUnique(GridItemWidget);
			}
		}
	}
}

int32 UGridInvSys_ContainerGridWidget::GetContainerGridItemIndex(const FIntPoint Position) const
{
	return IsValidPosition(Position) ? Position.X * ContainerGridSize.Y + Position.Y : -1;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridWidget::GetContainerGridItem(FIntPoint Position) const
{
	const int32 Index = GetContainerGridItemIndex(Position);
	check(Index >= 0);
	return Cast<UGridInvSys_ContainerGridItemWidget>(ContainerGridItemPanel->GetChildAt(Index));
}

UGridInvSys_ContainerGridDropWidget* UGridInvSys_ContainerGridWidget::GetContainerGridDragItem(FIntPoint Position) const
{
	const int32 Index = GetContainerGridItemIndex(Position);
	check(Index >= 0);
	return Cast<UGridInvSys_ContainerGridDropWidget>(ContainerGridDropPanel->GetChildAt(Index));
}

FName UGridInvSys_ContainerGridWidget::GetContainerGridID() const
{
	return ContainerGridID;
}

void UGridInvSys_ContainerGridWidget::SetContainerGridID(FName NewContainerGridID)
{
	ContainerGridID = NewContainerGridID;
}

bool UGridInvSys_ContainerGridWidget::IsValidPosition(const FIntPoint Position) const
{
	return !(Position.X < 0 || Position.Y < 0 || Position.X >= ContainerGridSize.X || Position.Y >= ContainerGridSize.Y);
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

bool UGridInvSys_ContainerGridWidget::NativeOnDragOver(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
	if (const UGridInvSys_InventoryItemInfo* ItemInfo = Cast<UGridInvSys_InventoryItemInfo>(InOperation->Payload))
	{
		const FIntPoint OriginPosition = CalculateGridOriginPoint(LocalPosition, ItemInfo->ItemSize);
		if (OriginPosition == LastDropOriginPosition)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}
		// 将上一帧的效果清除。
		ResetDragDropData();
		LastDropOriginPosition = OriginPosition;
		// 获取位置范围下的所有网格项
		GetContainerGridDragItems<UGridInvSys_ContainerGridDropWidget>(LastDropOverItems, OriginPosition, ItemInfo->ItemSize);
		const bool bIsCanDropItem = IsCanDropItem(OriginPosition, ItemInfo->ItemSize);
		for (UGridInvSys_ContainerGridDropWidget* DropOverItem : LastDropOverItems)
		{
			DropOverItem->NativeOnDraggingHovered(bIsCanDropItem);
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

void UGridInvSys_ContainerGridWidget::NativeOnDragEnter(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ResetDragDropData();
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
}

void UGridInvSys_ContainerGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	ResetDragDropData();
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

void UGridInvSys_ContainerGridWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	ResetDragDropData();
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

bool UGridInvSys_ContainerGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	ResetDragDropData();
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UGridInvSys_ContainerGridWidget::ResetDragDropData()
{
	for (UGridInvSys_ContainerGridDropWidget* LastOverItem : LastDropOverItems)
	{
		LastOverItem->NativeOnEndDraggingHovered();
	}
	LastDropOverItems.Empty();
	LastDropOriginPosition = FIntPoint(-1, -1);
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
	if (ContainerGridItemPanel->HasAnyChildren() == true)
	{
		ContainerGridItemPanel->ClearChildren();
		ContainerGridDropPanel->ClearChildren();
	}

	for (int i = 0; i < ContainerGridSize.X * ContainerGridSize.Y; ++i)
	{
		auto GridItemWidget = CreateWidget<UGridInvSys_ContainerGridItemWidget>(this, GridItemWidgetClass);
		auto GridDragWidget = CreateWidget<UGridInvSys_ContainerGridDropWidget>(this, GridDropWidgetClass);

		GridItemWidget->SetSlotName(NewSlotName);
		GridItemWidget->SetGridID(ContainerGridID);

		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		ContainerGridItemPanel->AddChildToGrid(GridItemWidget, SlotRow,SlotColumn);
		ContainerGridDropPanel->AddChildToUniformGrid(GridDragWidget, SlotRow,SlotColumn);
		
		GridItemWidget->InitContainerGridItem(this, FIntPoint(SlotRow, SlotColumn));
	}

	SlotName = NewSlotName;
}

void UGridInvSys_ContainerGridWidget::AddInventoryItemTo(const FGridInvSys_InventoryItem& InventoryItem)
{
	const FIntPoint ToPosition = InventoryItem.ItemPosition.Position; 
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

bool UGridInvSys_ContainerGridWidget::HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UWidget*>& Ignores)
{
	if (IsValidPosition(IntPoint) == false)
	{
		return false;
	}
	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(OutArray, IntPoint, ItemSize);
	if (OutArray.IsEmpty())
	{
		return true;
	}
	// 判断目标格子是否被占据，且不是需要忽略的格子。
	for (const UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
	{
		if (GridItemWidget->IsOccupied() && Ignores.Contains(GridItemWidget) == false)
		{
			return false;
		}
	}
	return true;
}

void UGridInvSys_ContainerGridWidget::GetContainerGridItemIndexes(TArray<int32>& OutArray, const FIntPoint Position, const FIntPoint Size) const
{
	OutArray.Empty();
	OutArray.Reserve(Size.X * Size.Y);
	for (int i = 0; i < Size.X; ++i)
	{
		for (int j = 0; j < Size.Y; ++j)
		{
			int32 Index = GetContainerGridItemIndex(FIntPoint(Position.X + i, Position.Y + j));
			// 仅添加正确位置的索引
			if (Index >= 0) OutArray.Add(Index);
		}
	}
}

FIntPoint UGridInvSys_ContainerGridWidget::CalculateGridOriginPoint(const FVector2D LocalPosition, const FIntPoint ItemSize) const
{
	const UGridInvSys_InventorySystemConfig* SystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>();
	check(SystemConfig);
	// 当前坐标 + ItemSize / 2 * 64
	int32 X = LocalPosition.Y / SystemConfig->ItemDrawSize;
	int32 Y = LocalPosition.X / SystemConfig->ItemDrawSize;
	// 判断行数是否为偶数
	if (ItemSize.X % 2 == 0)
	{
		if (LocalPosition.Y > X * SystemConfig->ItemDrawSize + SystemConfig->ItemDrawSize / 2)
		{
			++X;
		}
	}
	// 判断列数是否为偶数
	if (ItemSize.Y % 2 == 0)
	{
		if (LocalPosition.X > Y * SystemConfig->ItemDrawSize + SystemConfig->ItemDrawSize / 2)
		{
			++Y;
		}
	}
	// 得到正确的左上角的坐标
	return FIntPoint(X - FMath::Floor(ItemSize.X / 2), Y - FMath::Floor(ItemSize.Y / 2));
}
