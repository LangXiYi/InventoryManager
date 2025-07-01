// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/GridInvSys_GridInventoryControllerComponent.h"
#include "Components/GridPanel.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_ContainerGridDropWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_DragDropWidget.h"
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
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridItemWidget>(ContainerGridItemPanel->GetChildAt(Index)) : nullptr;
}

UGridInvSys_ContainerGridDropWidget* UGridInvSys_ContainerGridWidget::GetContainerGridDropItem(FIntPoint Position) const
{
	const int32 Index = GetContainerGridItemIndex(Position);
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridDropWidget>(ContainerGridDropPanel->GetChildAt(Index)) : nullptr;
}

bool UGridInvSys_ContainerGridWidget::FindValidPosition(FIntPoint ItemSize, FIntPoint& OutPosition, TArray<UWidget*> Ignores) const
{
	TArray<UGridInvSys_ContainerGridItemWidget*> FreeGridItemWidgets;
	FindAllFreeGridItems(FreeGridItemWidgets);

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

bool UGridInvSys_ContainerGridWidget::IsCanDropItemFromContainer(const UDragDropOperation* InOperation, FIntPoint ToPosition) const
{
#pragma region 常规检查
	check(InOperation);
	const UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload);
	if (!Payload) return false; 

	// 拖拽的目标必须是其他容器的成员，不能是装备槽或其他方式。
	const EDragDropType DragDropType = Payload->GetDragDropType();
	if (DragDropType != EDragDropType::Container) return false;

	UGridInvSys_ContainerGridItemWidget* FromGridItemWidget = Payload->GetGridItemWidget();
	if (!FromGridItemWidget) return false;
	
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	if (!ToGridItemWidget) return false;

	const FIntPoint FromItemSize = FromGridItemWidget->GetItemSize();
	// 判断 ToContainer 容器是否足够容纳 From
	if (!IsInRange(ToPosition, FromItemSize, FIntPoint(0, 0), ContainerGridSize))
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
		UGridInvSys_InventoryItemInfo* TempItemInfo = ItemWidget->GetItemInfo<UGridInvSys_InventoryItemInfo>();
		if (TempItemInfo)
		{
			FIntPoint TempOriginPosition = ItemWidget->GetOriginPosition();
			// 检查 OriginPos 是否在范围内
			if (!IsInRange(TempOriginPosition, TempItemInfo->ItemSize, ToPosition, FromItemSize) &&
				!IsInRange(ToPosition, FromItemSize, TempOriginPosition, TempItemInfo->ItemSize))
			{
				UE_LOG(LogInventorySystem, Log, TEXT("有物体在范围外 [%d]"), FromGridItemWidget->GetOccupiedGridItems().Num())
				return false;
			}
		}
	}

	// To 未被占领或是 From 与 To 的 Occupant 是同一对象
	if (ToGridItemWidget->IsOccupied() == false ||
		FromGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget->GetOriginGridItemWidget())
	{
		if (HasEnoughFreeSpace(ToPosition, FromItemSize, FromGridItemWidget->GetOccupiedGridItems()))
		{
			return true;
		}
		UE_LOG(LogInventorySystem, Log, TEXT("目标位置为空"))
	}

	// To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角
	if (FromItemSize == ToGridItemWidget->GetItemSize() && ToGridItemWidget == ToGridItemWidget->GetOriginGridItemWidget())
	{
		return true;
	}

	// To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小
	if (FromItemSize.X >= ToGridItemWidget->GetItemSize().X && FromItemSize.Y >= ToGridItemWidget->GetItemSize().Y)
	{
		return true;
	}
	// To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小
	if (FromGridItemWidget->GetContainerGridWidget() == ToGridItemWidget->GetContainerGridWidget())
	{
		FIntPoint Position;
		TArray<UWidget*> Ignores = FromGridItemWidget->GetOccupiedGridItems();
		Ignores.Append(ToGridItemWidget->GetOccupiedGridItems());
		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		return FindValidPosition(ToGridItemWidget->GetItemSize(), Position, Ignores);
	}
	return false;
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
	if (const UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload))
	{
		const UGridInvSys_InventoryItemInfo* ItemInfo = Payload->GetItemInfo<UGridInvSys_InventoryItemInfo>();
		if (ItemInfo == nullptr)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}
		
		const FIntPoint OriginPosition = CalculateGridOriginPoint(LocalPosition, ItemInfo->ItemSize);
		if (OriginPosition == LastDropOriginPosition)
		{
			return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
		}
		// 将上一帧的效果清除。
		ResetDragDropData();
		LastDropOriginPosition = OriginPosition;

		const bool bIsCanDropItem = IsCanDropItemFromContainer(InOperation, OriginPosition);
		
		// 获取位置范围下的所有网格项
		GetContainerGridDragItems<UGridInvSys_ContainerGridDropWidget>(LastDropOverItems, OriginPosition, ItemInfo->ItemSize);
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
	UGridInvSys_ContainerGridItemWidget* OriginGridItemWidget = Payload->GetGridItemWidget();
	const FIntPoint ToPosition = CalculateGridOriginPoint(LocalPosition, ItemInfo->ItemSize);
	if (OriginGridItemWidget->GetOriginPosition() == ToPosition)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);;
	}
	if (GetInventoryComponent() == Payload->GetInventoryComponent())
	{
		// 根据标记确认玩家拖拽的目标所在的位置
		switch (DragDropTypeEnum[InOperation->Tag])
		{
			case EDragDropType::Container:
				if (IsCanDropItemFromContainer(InOperation, ToPosition) == false) // 检查是否可以放置物品。
				{
					return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
				}
				OnDropItemFromContainer(OriginGridItemWidget, ToPosition);
				break;
			case EDragDropType::Equipment:
				// 从武器到容器
				break;
			default: ;
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

void UGridInvSys_ContainerGridWidget::OnDropItemFromContainer(UGridInvSys_ContainerGridItemWidget* FromGridItemWidget,
	FIntPoint ToPosition)
{
	UGridInvSys_GridInventoryControllerComponent* ControllerInvComp =
		GetOwningPlayer()->GetComponentByClass<UGridInvSys_GridInventoryControllerComponent>();
	check(ControllerInvComp);

	// 被修改的物品的唯一ID
	TArray<FName> ChangedItems;
	// 记录修改之后物品的新信息
	TArray<FGridInvSys_InventoryItem> NewItemsData;

	// 获取 From 在容器内占据的所有网格
	const TArray<UWidget*> FromGridItems = FromGridItemWidget->GetOccupiedGridItems();
	// 查询目标位置的范围内所有已经被占据网格，注意：这里会忽略 From 位置下的网格。
	TArray<UGridInvSys_ContainerGridItemWidget*> ToGridItems_Occupied;
	GetOccupiedGridItems(ToGridItems_Occupied, ToPosition, FromGridItemWidget->GetItemSize(), FromGridItems);

	FGridInvSys_InventoryItem FromItemData = FromGridItemWidget->GetInventoryItemData();
	FromItemData.ItemPosition.Position = ToPosition;
	NewItemsData.Add(FromItemData);
	ChangedItems.Add(FromItemData.BaseItemData.UniqueID);

	// 目标位置为空，则直接发送请求至服务器
	if (ToGridItems_Occupied.IsEmpty())
	{
		ControllerInvComp->Server_UpdateInventoryItems(ChangedItems, NewItemsData);
		return;
	}
	
	// 记录所有被修改的物品
	for (UGridInvSys_ContainerGridItemWidget* Widget : ToGridItems_Occupied)
	{
		if (ChangedItems.Contains(Widget->GetItemUniqueID()))
		{
			continue; // 忽略已经处理的物品
		}
		if (this == FromGridItemWidget->GetContainerGridWidget())
		{
			// 计算相较于 ToGridItemWidget 的相对坐标
			FIntPoint RelativePosition = Widget->CalculateRelativePosition(GetContainerGridItem(ToPosition));
			
			// 转换为对角坐标
			FIntPoint DiagonalPosition = FromGridItemWidget->GetOriginPosition();
			DiagonalPosition.X += FromGridItemWidget->GetItemSize().X - RelativePosition.X - Widget->GetItemSize().X;
			DiagonalPosition.Y += FromGridItemWidget->GetItemSize().Y - RelativePosition.Y - Widget->GetItemSize().Y;

			FGridInvSys_InventoryItem TempItemData = Widget->GetInventoryItemData();
			TempItemData.ItemPosition.Position = DiagonalPosition;
			NewItemsData.Add(TempItemData);
		}
		else
		{
			// 如果是不同的容器，XXX
		}
		// 记录已修改的物品唯一ID
		ChangedItems.Add(Widget->GetItemUniqueID());
	}
	// ControllerInvComp->Server_UpdateInventoryItems(ChangedItems, NewItemsData);

	
	

	
	
	
	return;
	/*
#pragma region 常规检查
	check(IsValidPosition(ToPosition))
	check(FromGridItemWidget)
	
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetContainerGridItem(ToPosition);
	const FIntPoint FromPosition = FromGridItemWidget->GetPosition();
	
	UE_LOG(LogInventorySystem, Log, TEXT("[交换物品] From [%d, %d] To [%d, %d].")
		, FromPosition.X, FromPosition.Y, ToPosition.X, ToPosition.Y);

	//UGridInvSys_ContainerGridWidget* FromContainerGrid = FromGridItemWidget->GetContainerGridWidget();
	UGridInvSys_ContainerGridWidget* ToContainerGrid = ToGridItemWidget->GetContainerGridWidget();
	check(FromContainerGrid);
	check(ToContainerGrid);

	UGridInvSys_InventoryItemInfo* ItemInfo_From = FromGridItemWidget->GetItemInfo<UGridInvSys_InventoryItemInfo>();
	check(ItemInfo_From);

	const FIntPoint ToContainerSize = ToContainerGrid->GetContainerGridSize();
	const FIntPoint FromItemSize = ItemInfo_From->ItemSize;
	// 判断 ToContainer 容器是否足够容纳 From
	if (ToContainerSize.X < ToPosition.X + FromItemSize.X || ToContainerSize.Y < ToPosition.Y + FromItemSize.Y)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("交换物品失败：ToContainer 无法在位置 [%d, %d] 容纳大小为 [%d, %d] 的物品。")
			, ToPosition.X, ToPosition.Y, FromItemSize.X, FromItemSize.Y);
		return;
	}

	// 限制条件，只有当 From 物品完全包裹 To 的所有物品 或是 To 完全包裹 From的物品 时才能交换物品。
	TArray<UGridInvSys_ContainerGridItemWidget*> OutWidgets;
	GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(OutWidgets, ToPosition, FromItemSize);
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : OutWidgets)
	{
		UGridInvSys_InventoryItemInfo* TempItemInfo = ItemWidget->GetItemInfo<UGridInvSys_InventoryItemInfo>();
		if (TempItemInfo)
		{
			FIntPoint TempOriginPosition = ItemWidget->GetOriginPosition();
			// 检查 OriginPos 是否在范围内
			if (IsInRange(TempOriginPosition, TempItemInfo->ItemSize, ToPosition, FromItemSize) == false)
			{
				// 如果包裹不住，判断该突出的物品能否把 FromItem 包裹住
				if (IsInRange(ToPosition, FromItemSize, TempOriginPosition, TempItemInfo->ItemSize) == false)
				{
					UE_LOG(LogInventorySystem, Warning, TEXT("交换物品失败：位置 [%d, %d] 处，存在范围外的对象。"), ToPosition.X, ToPosition.Y);
					return;
				}
			}
		}
	}
	
	UGridInvSys_InventoryItemInfo* ItemInfo_To = ToGridItemWidget->GetItemInfo<UGridInvSys_InventoryItemInfo>();
	check(ItemInfo_To);
	FIntPoint ToItemSize = ItemInfo_To->ItemSize;
#pragma endregion

#pragma region To 未被占领或是 From 与 To 的 Occupant 是同一对象
	if (ToGridItemWidget->IsOccupied() == false || ItemInfo_From == ItemInfo_To)
	{
		TArray<UWidget*> IgnoreItems;
		GetContainerGridItems(IgnoreItems, FromPosition, FromItemSize);
		if (ToContainerGrid->HasEnoughFreeSpace(ToPosition, FromItemSize, IgnoreItems))
		{
			UE_LOG(LogInventorySystem, Log, TEXT("交换物品成功：位置 [%d, %d] 未被其他物品占据。"), ToPosition.X, ToPosition.Y);
			FromGridItemWidget->RemoveItemInfo();
			ToGridItemWidget->UpdateItemInfo(ItemInfo_From);
			return;
		}
	}*/
#pragma endregion	
/*
#pragma region From 与 To 的大小一致 且 To 的位置是物品的左上角
	if (FromItemSize == ToItemSize && ToGridItemWidget == ToGridItemWidget_Origin)
	{
		// 两对象大小一致，且 ToPosition 是物品的锚点时，直接替换各自的 Occupant
		UE_LOG(InventorySystem, Log, TEXT("交换物品成功：两物品大小一致，且 ToPosition 是物品的锚点。"))
		FromGridItemWidget->RemoveOccupant();
		ToGridItemWidget->RemoveOccupant();
		FromGridItemWidget->UpdateOccupant(Occupant_To, true);
		ToGridItemWidget->UpdateOccupant(Occupant_From, true);
		return true;
	}
#pragma endregion

#pragma region 不同容器下的物品交换
	if (FromContainerGrid != ToContainerGrid)
	{
		//TArray<UObject*> OutItems;
		//ToContainerGrid->GetInventoryItemsByAre(OutItems, ToPosition, FromItemSize);
		
		TArray<FIntPoint> OutPosition;
		if(FromContainerGrid->HasEnoughFreeSpaceForArray(OutItemsInRange, OutPosition, {Occupant_From}))
		{
			UE_LOG(InventorySystem, Log, TEXT("To 位置可以容纳 From 对象"))
			// 先将 From 移除，然后在把 ToContainer 中的物品转移到 FromContainer
			FromGridItemWidget->RemoveOccupant();
			for (int i = 0; i < OutPosition.Num(); ++i)
			{
				if (IInventoryItemInterface* TempItemInterface = Cast<IInventoryItemInterface>(OutItemsInRange[i]))
				{
					// 获取 ToContainer 下的物品槽
					if (UContainerGridItemWidget* TempGridItemWidget = TempItemInterface->GetOriginGridItemWidget())
					{
						// 同样先移除再添加
						TempGridItemWidget->RemoveOccupant();
						FromContainerGrid->AddInventoryItemTo(OutItemsInRange[i], OutPosition[i], true);
					}
				}
			}
			// 最后将 From 物品添加到 ToContainer，因为之前已经调用 FromGridItemWidget 的移除函数，所以无需再次调用。
			// ToContainerGrid->AddInventoryItemTo(Occupant_From, ToPosition);
			ToGridItemWidget->UpdateOccupant(Occupant_From, true);
			return true;
		}
	}
#pragma endregion 

#pragma region 相同容器下的物品交换
	if (FromContainerGrid == ToContainerGrid)
	{
		if (FromItemSize.X >= ToItemSize.X && FromItemSize.Y >= ToItemSize.Y)
		{
			FromGridItemWidget->RemoveOccupant();
			// 计算所有物品相对与 ToPosition 的位置，然后将其转换为相对的对角位置，在将所有物品添加到 From 位置 + 相对的对角位置
			// TArray<UObject*> OutInventoryItems;
			// ToContainerGrid->GetInventoryItemsByAre(OutInventoryItems, ToPosition, FromItemSize, {Occupant_From});
			for (UObject* OutInventoryItem : OutItemsInRange)
			{
				if (IInventoryItemInterface* TempItemInterface = Cast<IInventoryItemInterface>(OutInventoryItem))
				{
					if (UContainerGridItemWidget* TempOriginGridItemWidget = TempItemInterface->GetOriginGridItemWidget())
					{
						// 计算相较于 ToGridItemWidget 的相对坐标
						FIntPoint TempRelativePosition;
						TempOriginGridItemWidget->CalculateRelativePosition(ToGridItemWidget, TempRelativePosition);

						// 转换为对角坐标
						TempRelativePosition.X = FromItemSize.X - TempRelativePosition.X - TempItemInterface->GetItemSize().X;
						TempRelativePosition.Y = FromItemSize.Y - TempRelativePosition.Y - TempItemInterface->GetItemSize().Y;

						// 根据相对的对角坐标，将物品添加到指定位置
						TempOriginGridItemWidget->RemoveOccupant();
						ToContainerGrid->AddInventoryItemTo(
							OutInventoryItem,
							FIntPoint(FromPosition.X + TempRelativePosition.X, FromPosition.Y + TempRelativePosition.Y),
							true);
					}
				}
			}
			ToGridItemWidget->UpdateOccupant(Occupant_From, true);
			return true;
		}
		else
		{
			// from size 小于 to size
			// 临时清除占据，from 占据 to 位置，判断 to 在容器内是否存在其他可放置的位置。
			// ToContainerGrid->HasEnoughFreeSpaceByPosition();
			// ToGridItemWidget->RemoveOccupant();
			ToGridItemWidget_Origin->RemoveOccupant();
			FromGridItemWidget->RemoveOccupant();

			ToGridItemWidget->UpdateOccupant(Occupant_From, true);
			
			UContainerGridItemWidget* FreeGridItemWidget;
			if (ToContainerGrid->HasEnoughFreeGridItems(ToItemSize,FreeGridItemWidget))
			{
				FreeGridItemWidget->UpdateOccupant(Occupant_To, true);
				return true;
			}
			else
			{
				UE_LOG(InventorySystem, Warning, TEXT("交换物品失败：容器内无法继续放置 [%d, %d] 大小的物品，现在开始还原容器内容。"), ToItemSize.X, ToItemSize.Y);
				// 还原 ToGridItemWidget 与 FromGridItemWidget
				ToGridItemWidget->RemoveOccupant();
				// FromGridItemWidget->RemoveOccupant();
				ToGridItemWidget_Origin->UpdateOccupant(Occupant_To, true);
				FromGridItemWidget->UpdateOccupant(Occupant_From, true);
				return false;
			}
		}
	}
#pragma endregion*/

	// fixed: 小物品换大物品存在问题
	//UE_LOG(InventorySystem, Warning, TEXT("转移物品失败, FromContainer 无法容纳所有物品。"))
	//return false;
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
		GridItemWidget->SetInventoryComponent(GetInventoryComponent());

		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		ContainerGridItemPanel->AddChildToGrid(GridItemWidget, SlotRow,SlotColumn);
		ContainerGridDropPanel->AddChildToUniformGrid(GridDragWidget, SlotRow,SlotColumn);
		
		GridItemWidget->SetContainerGridWidget(this);
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
		ToGridItemWidget->UpdateItemInfo(InventoryItem);
	}
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
	RemoveGridItemWidget->RemoveItemInfo();
}

bool UGridInvSys_ContainerGridWidget::HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UWidget*>& Ignores) const
{
	if (IsInRange(IntPoint, ItemSize, FIntPoint(0, 0), ContainerGridSize) == false)
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

void UGridInvSys_ContainerGridWidget::FindAllFreeGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray) const
{
	OutArray.Empty();
	TArray<int32> OutIndexes;
	GetContainerGridItemIndexes(OutIndexes, FIntPoint(0, 0), ContainerGridSize);
	for (const int32 Index : OutIndexes)
	{
		UGridInvSys_ContainerGridItemWidget* GridItemWidget = Cast<UGridInvSys_ContainerGridItemWidget>(ContainerGridItemPanel->GetChildAt(Index));
		if (GridItemWidget->IsOccupied() == false)
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
