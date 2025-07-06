// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/GridInvSys_GridInventoryControllerComponent.h"
#include "Components/GridPanel.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridDropWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_DragDropWidget.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
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

void UGridInvSys_ContainerGridWidget::UpdateContainerGridSize(FIntPoint GridSize)
{
	ContainerGridSize = GridSize;
	int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize * GridSize.Y);
		SizeBox->SetHeightOverride(ItemDrawSize * GridSize.X);
	}
}

int32 UGridInvSys_ContainerGridWidget::GetContainerGridItemIndex(const FIntPoint Position) const
{
	return IsValidPosition(Position) ? Position.X * ContainerGridSize.Y + Position.Y : -1;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridWidget::GetContainerGridItem(FIntPoint Position) const
{
	const int32 Index = GetContainerGridItemIndex(Position);
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridItemWidget>(ContainerGridItemPanel->GetChildAt(Index)) : nullptr;
}

UGridInvSys_ContainerGridDropWidget* UGridInvSys_ContainerGridWidget::GetContainerGridDropItem(FIntPoint Position) const
{
	const int32 Index = GetContainerGridItemIndex(Position);
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridDropWidget>(ContainerGridDropPanel->GetChildAt(Index)) : nullptr;
}

bool UGridInvSys_ContainerGridWidget::FindValidPosition(FIntPoint ItemSize, FIntPoint& OutPosition, const TArray<UWidget*>& Ignores) const
{
	TArray<UGridInvSys_ContainerGridItemWidget*> FreeGridItemWidgets;
	FindAllFreeGridItems(FreeGridItemWidgets, Ignores);

	for (UGridInvSys_ContainerGridItemWidget* FreeGridItemWidget : FreeGridItemWidgets)
	{
		OutPosition = FreeGridItemWidget->GetPosition();
		if (HasEnoughFreeSpace(OutPosition, ItemSize, Ignores) == true)
		{
			OutPosition = FreeGridItemWidget->GetPosition();
			return true;
		}
	}
	OutPosition = FIntPoint(-1, -1);
	return false;
}

FName UGridInvSys_ContainerGridWidget::GetContainerGridID() const
{
	return ContainerGridID;
}

FName UGridInvSys_ContainerGridWidget::GetSlotName() const
{
	return SlotName;
}

void UGridInvSys_ContainerGridWidget::SetContainerGridID(FName NewContainerGridID)
{
	ContainerGridID = NewContainerGridID;
}

bool UGridInvSys_ContainerGridWidget::IsValidPosition(const FIntPoint Position) const
{
	return !(Position.X < 0 || Position.Y < 0 || Position.X >= ContainerGridSize.X || Position.Y >= ContainerGridSize.Y);
}

bool UGridInvSys_ContainerGridWidget::IsInRangePosition(FIntPoint TargetPos, FIntPoint OriginPos, FIntPoint Size) const
{
	if (IsValidPosition(OriginPos) &&
		IsValidPosition(FIntPoint(OriginPos.X + Size.X - 1, OriginPos.Y + Size.Y - 1)))
	{
		return (TargetPos.X >= OriginPos.X && TargetPos.X <= OriginPos.X + Size.X - 1)
			&& (TargetPos.Y >= OriginPos.Y && TargetPos.Y <= OriginPos.Y + Size.Y - 1);
	}
	return false;
}

bool UGridInvSys_ContainerGridWidget::IsInRange(FIntPoint TargetPos, FIntPoint TargetSize, FIntPoint OriginPos,
	FIntPoint Size) const
{
	return IsInRangePosition(TargetPos, OriginPos, Size)
		&& IsInRangePosition(FIntPoint(TargetPos.X + TargetSize.X - 1, TargetPos.Y + TargetSize.Y - 1), OriginPos, Size);
}

bool UGridInvSys_ContainerGridWidget::IsInContainer(FIntPoint TargetPos, FIntPoint TargetSize) const
{
	return IsInRange(TargetPos, TargetSize, FIntPoint(0, 0), ContainerGridSize);
}

bool UGridInvSys_ContainerGridWidget::IsCanDropItemFromContainer(UGridInvSys_ContainerGridItemWidget* FromGridItemWidget,
	FIntPoint ToPosition,  FIntPoint FromItemSize) const
{
#pragma region 常规检查
	/*check(InOperation);
	const UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload);
	if (!Payload) return false; 

	// 拖拽的目标必须是其他容器的成员，不能是装备槽或其他方式。
	const EDragDropType DragDropType = Payload->GetDragDropType();
	if (DragDropType != EDragDropType::Container) return false;

	UGridInvSys_ContainerGridItemWidget* FromGridItemWidget = Payload->GetGridItemWidget();*/
	if (!FromGridItemWidget) return false;
	
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	if (!ToGridItemWidget) return false;

	/*FIntPoint FromItemSize = FromGridItemWidget->GetItemSize();
	if (Direction == EGridInvSys_ItemDirection::Vertical)
	{
		FromItemSize = FIntPoint(FromItemSize.Y, FromItemSize.X);
	}*/
	// 判断物品在容器内
	if (!IsInContainer(ToPosition, FromItemSize))
	{
		return false;
	}
#pragma endregion

	// 收集所有被FromItem覆盖的网格
	TArray<UGridInvSys_ContainerGridItemWidget*> OutWidgets;
	GetOccupiedGridItems(OutWidgets, ToPosition, FromItemSize, FromGridItemWidget->GetOccupiedGridItems());
	// 限制条件，只有当 From 物品完全包裹 To 的所有物品 或是 To 完全包裹 From的物品 时才能交换物品。
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : OutWidgets)
	{
		FIntPoint TempOriginPosition = ItemWidget->GetOriginPosition();
		// 检查 OriginPos 是否在范围内
		if (!IsInRange(TempOriginPosition, ItemWidget->GetItemSize(), ToPosition, FromItemSize) &&
			!IsInRange(ToPosition, FromItemSize, TempOriginPosition, ItemWidget->GetItemSize()))
		{
			UE_LOG(LogInventorySystem, Log, TEXT("有物体在范围外 [%d]"), FromGridItemWidget->GetOccupiedGridItems().Num())
			return false;
		}
	}

	// To 未被占领或是 From 与 To 的 Occupant 是同一对象
	if (ToGridItemWidget->IsOccupied() == false ||
		FromGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget->GetOriginGridItemWidget())
	{
		if (HasEnoughFreeSpace(ToPosition, FromItemSize, FromGridItemWidget->GetOccupiedGridItems()))
		{
			UE_LOG(LogInventorySystem, Log, TEXT("To 未被占领或是 From 与 To 的 Occupant 是同一对象"))
			return true;
		}
	}

	// To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角
	if (FromItemSize == ToGridItemWidget->GetItemSize() && ToGridItemWidget == ToGridItemWidget->GetOriginGridItemWidget())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角"))
		return true;
	}

	// 物品发生旋转时，范围内不能存在其他物品。
	if (FromItemSize != FromGridItemWidget->GetItemSize())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("物品发生旋转时，范围内不能存在其他物品。"))
		return HasEnoughFreeSpace(ToPosition, FromItemSize, FromGridItemWidget->GetOccupiedGridItems());
	}

	// To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小
	if (FromItemSize.X >= ToGridItemWidget->GetItemSize().X && FromItemSize.Y >= ToGridItemWidget->GetItemSize().Y)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小"))
		return true;
	}
	// To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小
	if (FromGridItemWidget->GetContainerGridWidget() == ToGridItemWidget->GetContainerGridWidget())
	{
		FIntPoint Position;
		TArray<UWidget*> Ignores = FromGridItemWidget->GetOccupiedGridItems();
		Ignores.Append(ToGridItemWidget->GetOccupiedGridItems());
		TArray<UWidget*> NotIgnores;
		GetContainerGridItems(NotIgnores, ToPosition, FromItemSize);
		// 被From占据的网格不会被忽略
		for (UWidget* NotIgnore : NotIgnores)
		{
			Ignores.Remove(NotIgnore);
		}
		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小"))
		return FindValidPosition(ToGridItemWidget->GetItemSize(), Position, Ignores);
	}
	return false;
}

void UGridInvSys_ContainerGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	/*UpdateContainerGridSize(ContainerGridSize);
	
	int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize * ContainerGridSize.Y);
		SizeBox->SetHeightOverride(ItemDrawSize * ContainerGridSize.X);
	}*/
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

int32 UGridInvSys_ContainerGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 创建控件的线条
	//FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	/*SizeBox->
	// MyCullingRect->
	// 获取当前组件在屏幕控件的坐标
	const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D LocalPosition = AllottedGeometry.AbsoluteToLocal(ScreenPosition);
	// DrawDashedLine()
	UWidgetBlueprintLibrary::DrawLine(Context, )*/
	
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                          bParentEnabled);
}

bool UGridInvSys_ContainerGridWidget::NativeOnDragOver(const FGeometry& InGeometry,
                                                       const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UGridInvSys_DragItemWidget* DefaultDragVisual = Cast<UGridInvSys_DragItemWidget>(InOperation->DefaultDragVisual);
	const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
	if (UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload))
	{
		const UGridInvSys_InventoryItemInfo* ItemInfo = Payload->GetItemInfo<UGridInvSys_InventoryItemInfo>();
		if (ItemInfo == nullptr)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}

		FIntPoint NormalItemSize = ItemInfo->ItemSize;
		FIntPoint NormalOriginPosition = CalculateGridOriginPoint(LocalPosition, NormalItemSize);
		
		FIntPoint TargetItemSize = NormalItemSize;
		FIntPoint TargetOriginPosition = NormalOriginPosition;
		
		if (NormalOriginPosition == LastDropOriginPosition)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}

		// 将上一帧的效果清除。
		ResetDragDropData();

		EDragDropType DragDropType = Payload->GetDragDropType();
		bool bIsCanDropItem = false;
		bool bIsNormalDirection = true;
		if (DragDropType == EDragDropType::Container)
		{
			bIsCanDropItem = IsCanDropItemFromContainer(Payload->GetGridItemWidget(), NormalOriginPosition, NormalItemSize);
			if (bIsCanDropItem == false && NormalItemSize.X != NormalItemSize.Y)
			{
				// 大小不同的物品在无法放置时会尝试旋转方向然后放置。
				bIsNormalDirection = false;
				TargetItemSize = FIntPoint(ItemInfo->ItemSize.Y, ItemInfo->ItemSize.X);
				TargetOriginPosition = CalculateGridOriginPoint(LocalPosition, TargetItemSize);
				UE_LOG(LogInventorySystem, Log, TEXT("Target Origin Position = [%d, %d]"), TargetOriginPosition.X, TargetOriginPosition.Y);
				bIsCanDropItem = IsCanDropItemFromContainer(Payload->GetGridItemWidget(),TargetOriginPosition, TargetItemSize);
			}
		}
		if (bIsNormalDirection == false && bIsCanDropItem)
		{
			LastDropOriginPosition = TargetOriginPosition;
			DefaultDragVisual->UpdateDirection(EGridInvSys_ItemDirection::Vertical);
			Payload->SetDirection(EGridInvSys_ItemDirection::Vertical);
			// 获取位置范围下的所有网格项
			GetContainerGridDragItems<UGridInvSys_ContainerGridDropWidget>(LastDropOverItems, TargetOriginPosition, TargetItemSize);
			for (UGridInvSys_ContainerGridDropWidget* DropOverItem : LastDropOverItems)
			{
				DropOverItem->NativeOnDraggingHovered(bIsCanDropItem);
			}
		}
		else
		{
			LastDropOriginPosition = NormalOriginPosition;
			DefaultDragVisual->UpdateDirection(EGridInvSys_ItemDirection::Horizontal);
			Payload->SetDirection(EGridInvSys_ItemDirection::Horizontal);
			// 获取位置范围下的所有网格项
			GetContainerGridDragItems<UGridInvSys_ContainerGridDropWidget>(LastDropOverItems, NormalOriginPosition, NormalItemSize);
			for (UGridInvSys_ContainerGridDropWidget* DropOverItem : LastDropOverItems)
			{
				DropOverItem->NativeOnDraggingHovered(bIsCanDropItem);
			}
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
	UGridInvSys_DragItemWidget* DragItemWidget = Cast<UGridInvSys_DragItemWidget>(InOperation->DefaultDragVisual);
	const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
	const UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload);
	if (Payload == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}
	const UGridInvSys_InventoryItemInfo* ItemInfo = Payload->GetItemInfo<UGridInvSys_InventoryItemInfo>();
	if (ItemInfo == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}
	UGridInvSys_ContainerGridItemWidget* FromItem = Payload->GetGridItemWidget();
	
	FIntPoint DragItemSize = DragItemWidget->GetItemSize(); // 拖拽物品的大小，会随方向改变。
	FIntPoint ToPosition = CalculateGridOriginPoint(LocalPosition, DragItemSize);
	EDragDropType DragDropType = Payload->GetDragDropType();

	// 构建数据
	FGridInvSys_InventoryItemPosition ToPositionData;
	ToPositionData.Position = ToPosition;
	ToPositionData.GridID = GetContainerGridID();
	ToPositionData.SlotName = GetSlotName();
	ToPositionData.Direction = DragItemWidget->GetDirection();
	ToPositionData.ItemSize = DragItemSize;

	// 在相同容器内若位置不变则不发生改变
	/*if (FromItem->GetOriginPosition() == ToPosition &&
		FromItem->GetContainerGridWidget() == this &&
		DragItemSize == FromItem->GetItemSize())
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);;
	}*/
	
	if (GetInventoryComponent() == Payload->GetInventoryComponent())
	{
		// 根据标记确认玩家拖拽的目标所在的位置
		if (DragDropType == EDragDropType::Container)
		{
			TryDropItemFromContainer(FromItem, DragItemSize, ToPositionData);
		}
	}
	else
	{
		//todo::在不同库存组件内操作
	}
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

bool UGridInvSys_ContainerGridWidget::TryDropItemFromContainer(UGridInvSys_ContainerGridItemWidget* FromGridItemWidget,
	FIntPoint FromItemSize,	FGridInvSys_InventoryItemPosition ItemPositionData) const
{
	FIntPoint ToPosition = ItemPositionData.Position;
	//FIntPoint FromItemSize = FromGridItemWidget->GetItemSize();
	if (IsCanDropItemFromContainer(FromGridItemWidget, ToPosition, FromItemSize) == false) // 检查是否可以放置物品。
	{
		return false;
		/*FromItemSize = FIntPoint(FromItemSize.Y, FromItemSize.X);
		if (FromItemSize.X != FromItemSize.Y &&
			IsCanDropItemFromContainer(FromGridItemWidget, ToPosition, FromItemSize) == false) // 检查是否可以放置物品。
		{
			return false;
		}*/
	}
	UE_LOG(LogInventorySystem, Log, TEXT("尝试放下物品至 [%s = %-2.2d:%-2.2d]"), *ItemPositionData.SlotName.ToString(), ToPosition.X, ToPosition.Y);
	UGridInvSys_GridInventoryControllerComponent* ControllerInvComp =
		GetOwningPlayer()->GetComponentByClass<UGridInvSys_GridInventoryControllerComponent>();
	check(ControllerInvComp);

	// 被修改的物品的唯一ID
	TArray<FName> ChangedItems;
	// 记录修改之后物品的新信息
	TArray<FGridInvSys_InventoryItemPosition> NewItemsData;

	// 获取 From 在容器内占据的所有网格
	const TArray<UWidget*> FromGridItems = FromGridItemWidget->GetOccupiedGridItems();
	// 查询目标位置的范围内所有已经被占据网格，注意：这里会忽略 From 位置下的网格。
	TArray<UGridInvSys_ContainerGridItemWidget*> ToOccupiedItems;
	GetOccupiedGridItems(ToOccupiedItems, ToPosition, FromItemSize, FromGridItems);

	// IsCanDrop == true 所以可以直接将 From 的位置修改为目标位置并记录下来。
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	//FGridInvSys_InventoryItem FromItemData = FromGridItemWidget->GetItemUniqueID();
	//FromItemData.ItemPosition = ItemPositionData;
	// todo::这里的判断太简陋了。已经修改为在上一级通过DragDrop控件获取旋转属性。
	// todo:: 旋转方向也要更新
	/*UGridInvSys_InventoryItemInfo* FromItemInfo = FromGridItemWidget->GetItemInfo<UGridInvSys_InventoryItemInfo>();
	if (FromItemInfo->ItemSize != FromItemSize)
	{
		ItemPositionData.Direction = EGridInvSys_ItemDirection::Vertical;
	}
	else
	{
		ItemPositionData.Direction = EGridInvSys_ItemDirection::Horizontal;
	}*/
	NewItemsData.Add(ItemPositionData);
	ChangedItems.Add(FromGridItemWidget->GetItemUniqueID());

	// To 未被占领或是 From 与 To 的 Occupant 是同一对象
	if (ToOccupiedItems.IsEmpty())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] To 未被占领或是 From 与 To 的 Occupant 是同一对象"))
		ControllerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
		return true;
	}

	// To 的位置已经被其他物品占领，但 From 与 To 的大小一致
	if (FromItemSize == ToGridItemWidget->GetItemSize() &&
		ToGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] From 与 To 的大小一致 且 To 的位置是物品的左上角"))
		// 交换双方位置
		NewItemsData.Add(FromGridItemWidget->GetGridItemPosition());
		ChangedItems.Add(ToGridItemWidget->GetItemUniqueID());
		ControllerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
		return true;
	}

	// 不同容器下的物品交换
	UGridInvSys_ContainerGridWidget* FromContainerGrid = FromGridItemWidget->GetContainerGridWidget();
	if (FromContainerGrid != this)
	{
		// From > To 且有足够的控件容纳该物品
		if (FromItemSize.X >= ToGridItemWidget->GetItemSize().X &&
			FromItemSize.Y >= ToGridItemWidget->GetItemSize().Y)
		{
			UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] 不同容器下的物品交换To 位置可以容纳 From 对象"))
			// 获取范围内所有物品，然后将其位置转移即可
			for (UGridInvSys_ContainerGridItemWidget* ToOccupiedItem : ToOccupiedItems)
			{
				if (ChangedItems.Contains(ToOccupiedItem->GetItemUniqueID()))
				{
					continue; // 忽略已经处理的物品
				}
				// 计算相对于目标位置的相对坐标
				FIntPoint RelativePosition = ToOccupiedItem->CalculateRelativePosition(ToGridItemWidget);
				// UE_LOG(LogInventorySystem, Log, TEXT("计算得到的相对坐标为 [%d,%d]"), RelativePosition.X, RelativePosition.Y);

				// 转换为相对From容器的坐标
				FIntPoint TargetPosition = FromGridItemWidget->GetOriginPosition();
				TargetPosition.X += RelativePosition.X;
				TargetPosition.Y += RelativePosition.Y;
				// UE_LOG(LogInventorySystem, Log, TEXT("计算得到的最终坐标为 [%d,%d]"), TargetPosition.X, TargetPosition.Y);
				// 计算这些ToOccupiedItems在From容器下的位置
				FGridInvSys_InventoryItemPosition TempItemData;
				TempItemData.Position = TargetPosition;
				TempItemData.SlotName = FromContainerGrid->GetSlotName();
				TempItemData.GridID = FromGridItemWidget->GetGridID();
				TempItemData.Direction = ToOccupiedItem->GetItemDirection(); // 方向保持不变。
				NewItemsData.Add(TempItemData);
				ChangedItems.Add(ToOccupiedItem->GetItemUniqueID());
			}
			ControllerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
			return true;
		}
	}

	// To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小
	// 注意：
	if (FromItemSize.X >= ToGridItemWidget->GetItemSize().X &&
		FromItemSize.Y >= ToGridItemWidget->GetItemSize().Y ||
		FromGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget->GetOriginGridItemWidget())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] 相同容器下交换，To 的位置已经被其他物品占领，且拖拽的物品大小 >= 放置位置的物品的大小"))
		// 获取范围内所有物品，然后将其位置转移即可
		for (UGridInvSys_ContainerGridItemWidget* ToOccupiedItem : ToOccupiedItems)
		{
			if (ChangedItems.Contains(ToOccupiedItem->GetItemUniqueID()))
			{
				continue; // 忽略已经处理的物品
			}
			// 计算相较于 ToGridItemWidget 的相对坐标
			// 注意：如果发生了旋转则相对位置会不准确，需要再转换一次
			FIntPoint RelativePosition = ToOccupiedItem->CalculateRelativePosition(GetContainerGridItem(ToPosition));
			
			// 转换为对角坐标
			FIntPoint DiagonalPosition = FromGridItemWidget->GetOriginPosition();
			DiagonalPosition.X += FromGridItemWidget->GetItemSize().X - RelativePosition.X - ToOccupiedItem->GetItemSize().X;
			DiagonalPosition.Y += FromGridItemWidget->GetItemSize().Y - RelativePosition.Y - ToOccupiedItem->GetItemSize().Y;
			UE_LOG(LogInventorySystem, Log, TEXT("对角坐标.X = %d + %d - %d - %d"),FromGridItemWidget->GetOriginPosition().X,
				FromItemSize.X, RelativePosition.X, ToOccupiedItem->GetItemSize().X)
			UE_LOG(LogInventorySystem, Log, TEXT("对角坐标.Y = %d + %d - %d - %d"), FromGridItemWidget->GetOriginPosition().Y,
				FromItemSize.Y, RelativePosition.Y, ToOccupiedItem->GetItemSize().Y)

			// 计算这些ToOccupiedItems在From容器下的位置
			FGridInvSys_InventoryItemPosition TempItemData;
			TempItemData.Position = DiagonalPosition;
			TempItemData.SlotName = FromContainerGrid->GetSlotName();
			TempItemData.GridID = FromGridItemWidget->GetGridID();
			TempItemData.Direction = ToOccupiedItem->GetItemDirection(); // 方向保持不变。
			NewItemsData.Add(TempItemData);
			ChangedItems.Add(ToOccupiedItem->GetItemUniqueID());
		}
		ControllerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
		return true;
	}
	else
	{
		// from size 小于 to size
		FIntPoint Position;
		TArray<UWidget*> Ignores = FromGridItemWidget->GetOccupiedGridItems();
		Ignores.Append(ToGridItemWidget->GetOccupiedGridItems());
		TArray<UWidget*> NotIgnores;
		GetContainerGridItems(NotIgnores, ToPosition, FromItemSize);
		// 被From占据的网格不会被忽略
		for (UWidget* NotIgnore : NotIgnores)
		{
			Ignores.Remove(NotIgnore);
		}
		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		UE_LOG(LogInventorySystem, Log, TEXT("[%d] To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小"), Ignores.Num())
		// 查找 To 物品 可以放置的位置
		if (FindValidPosition(ToGridItemWidget->GetItemSize(), Position, Ignores))
		{
			// 计算 To 物品在From容器下的位置
			FGridInvSys_InventoryItemPosition TempItemData;
			TempItemData.Position = Position;
			TempItemData.SlotName = FromContainerGrid->GetSlotName();
			TempItemData.GridID = FromGridItemWidget->GetGridID();
			TempItemData.Direction = ToGridItemWidget->GetItemDirection(); // 方向保持不变。
			NewItemsData.Add(TempItemData);
			ChangedItems.Add(ToGridItemWidget->GetItemUniqueID());
			ControllerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
			return true;
		}
	}
	return false;
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
	SlotName = NewSlotName;
	OnConstructItems();
	
	if (IsDesignTime())
	{
		return;
	}
	
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
		GridItemWidget->SetInventoryComponent(GetInventoryComponent());

		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		ContainerGridItemPanel->AddChildToGrid(GridItemWidget, SlotRow,SlotColumn);
		ContainerGridDropPanel->AddChildToUniformGrid(GridDragWidget, SlotRow,SlotColumn);
		
		GridItemWidget->SetContainerGridWidget(this);
	}
}

void UGridInvSys_ContainerGridWidget::UpdateInventoryItem(const FGridInvSys_InventoryItem& InventoryItem)
{
	const FIntPoint ToPosition = InventoryItem.ItemPosition.Position; 
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	if (ToGridItemWidget == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("添加物品失败：ToGridItemWidget is nullptr."));
		return;
	}
	ToGridItemWidget->UpdateItemInfo(InventoryItem);
}

void UGridInvSys_ContainerGridWidget::RemoveInventoryItem(const FGridInvSys_InventoryItem& InventoryItem)
{
	const FIntPoint ToPosition = InventoryItem.ItemPosition.Position;
	UGridInvSys_ContainerGridItemWidget* RemoveGridItemWidget = GetContainerGridItem(ToPosition);
	if (RemoveGridItemWidget == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("移除物品失败：RemoveGridItemWidget is nullptr."));
		return;
	}

	TArray<UGridInvSys_ContainerGridItemWidget*> OutArray;
	GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(OutArray,ToPosition, InventoryItem.ItemPosition.ItemSize);
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : OutArray)
	{
		// 判断要删除的物品唯一ID是否与需要删除的物品一致，避免误删。
		if (ItemWidget->GetItemUniqueID() == InventoryItem.BaseItemData.UniqueID)
		{
			ItemWidget->RemoveItemInfo();
		}
	}
}

void UGridInvSys_ContainerGridWidget::RemoveAllInventoryItem()
{
	TArray<UGridInvSys_ContainerGridItemWidget*> ItemWidgets = GetAllContainerGridItems();
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : ItemWidgets)
	{
		ItemWidget->RemoveItemInfo();
	}
}

bool UGridInvSys_ContainerGridWidget::HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UWidget*>& Ignores) const
{
	if (IsInContainer(IntPoint, ItemSize) == false)
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

void UGridInvSys_ContainerGridWidget::GetOccupiedGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray,
	FIntPoint Position, FIntPoint Size, const TArray<UWidget*>& Ignores) const
{
	TArray<UGridInvSys_ContainerGridItemWidget*> TempArray;
	GetContainerGridItems(TempArray, Position, Size, Ignores);
	for (UGridInvSys_ContainerGridItemWidget* Item : TempArray)
	{
		if (Item->IsOccupied())
		{
			OutArray.Add(Item);
		}
	}
}

void UGridInvSys_ContainerGridWidget::FindAllFreeGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, const TArray<UWidget*>& Ignores) const
{
	OutArray.Empty();
	TArray<int32> OutIndexes;
	TArray<UGridInvSys_ContainerGridItemWidget*> GridItemWidgets = GetAllContainerGridItems();
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : GridItemWidgets)
	{
		if (GridItemWidget->IsOccupied() == false || Ignores.Contains(GridItemWidget))
		{
			OutArray.AddUnique(GridItemWidget);
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

TArray<UGridInvSys_ContainerGridItemWidget*> UGridInvSys_ContainerGridWidget::GetAllContainerGridItems() const
{
	TArray<UGridInvSys_ContainerGridItemWidget*> TempArray;
	GetContainerGridItems(TempArray, FIntPoint(0, 0), ContainerGridSize);
	return TempArray;
}
