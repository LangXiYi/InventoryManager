// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/GridInvSys_GridInventoryControllerComponent.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/GridPanel.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Data/GridInvSys_InventoryItemInfo.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interface/GridInvSys_DraggingItemInterface.h"
#include "Interface/InvSys_DraggingItemInterface.h"
#include "Widgets/GridInvSys_ContainerGridDropWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
#include "Widgets/GridInvSys_InventoryItemWidget.h"
#include "Widgets/Notifications/SNotificationList.h"


void UGridInvSys_ContainerGridWidget::ConstructGridItems(int32 InGridID)
{
	if (GridItemWidgetClass == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("GridItemWidgetClass is nullptr."))
		return;
	}
	
	ContainerGridID = InGridID;

	//  清除 ContainerGrid 中所有子项。
	if (ContainerGridItemPanel->HasAnyChildren() == true)
	{
		ContainerGridItemPanel->ClearChildren();
		ContainerGridDropPanel->ClearChildren();
	}

	// 创建所有网格
	for (int i = 0; i < ContainerGridSize.X * ContainerGridSize.Y; ++i)
	{
		auto GridItemWidget = CreateWidget<UGridInvSys_ContainerGridItemWidget>(this, GridItemWidgetClass);
		auto GridDragWidget = CreateWidget<UGridInvSys_ContainerGridDropWidget>(this, GridDropWidgetClass);

		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		ContainerGridItemPanel->AddChildToGrid(GridItemWidget, SlotRow,SlotColumn);
		ContainerGridDropPanel->AddChildToUniformGrid(GridDragWidget, SlotRow,SlotColumn);

		GridItemWidget->SetInventoryComponent(InventoryComponent.Get());
		GridItemWidget->SetSlotTag(SlotTag);
		GridItemWidget->OnConstructGridItem(this, FIntPoint(SlotRow, SlotColumn));
	}
}

void UGridInvSys_ContainerGridWidget::FindContainerGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray,
                                                             FIntPoint Position, FIntPoint ItemSize, const TSet<UGridInvSys_ContainerGridItemWidget*>& Ignores)
{
	OutArray.Empty(ItemSize.X * ItemSize.Y);
	for (int i = 0; i < ItemSize.X; ++i)
	{
		for (int j = 0; j < ItemSize.Y; ++j)
		{
			UGridInvSys_ContainerGridItemWidget* GridItemWidget = GetGridItemWidget(FIntPoint(Position.X + i, Position.Y + j));
			// 仅处理 UContainerGridItemWidget 及其子类。
			if (GridItemWidget != nullptr && Ignores.Contains(GridItemWidget) == false)
			{
				OutArray.AddUnique(GridItemWidget);
			}
		}
	}
}

void UGridInvSys_ContainerGridWidget::UpdateContainerGridSize()
{
	/*int32 ItemDrawSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	if (SizeBox)
	{
		SizeBox->SetWidthOverride(ItemDrawSize * ContainerGridSize.Y);
		SizeBox->SetHeightOverride(ItemDrawSize * ContainerGridSize.X);
	}*/
}

int32 UGridInvSys_ContainerGridWidget::GetItemIndex(const FIntPoint Position) const
{
	return IsValidPosition(Position) ? Position.X * ContainerGridSize.Y + Position.Y : -1;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridWidget::GetGridItemWidget(FIntPoint Position) const
{
	const int32 Index = GetItemIndex(Position);
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridItemWidget>(ContainerGridItemPanel->GetChildAt(Index)) : nullptr;
}

UGridInvSys_ContainerGridDropWidget* UGridInvSys_ContainerGridWidget::GetContainerGridDropItem(FIntPoint Position) const
{
	const int32 Index = GetItemIndex(Position);
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

int32 UGridInvSys_ContainerGridWidget::GetContainerGridID() const
{
	return ContainerGridID;
}

FName UGridInvSys_ContainerGridWidget::GetSlotName() const
{
	return SlotName;
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

bool UGridInvSys_ContainerGridWidget::IsCanDropItemFromContainer(UGridInvSys_ContainerGridWidget* FromContainer, FIntPoint ToPosition,  FIntPoint FromItemSize) const
{
#pragma region 常规检查
	/*check(InOperation);
	const UGridInvSys_DragDropWidget* Payload = Cast<UGridInvSys_DragDropWidget>(InOperation->Payload);
	if (!Payload) return false; 

	// 拖拽的目标必须是其他容器的成员，不能是装备槽或其他方式。
	const EDragDropType DragDropType = Payload->GetDragDropType();
	if (DragDropType != EDragDropType::Container) return false;

	UGridInvSys_ContainerGridItemWidget* FromGridItemWidget = Payload->GetGridItemWidget();*/
	// if (!FromGridItemWidget) return false;
	
	UGridInvSys_ContainerGridItemWidget* ToOriginItemWidget = GetGridItemWidget(ToPosition);
	if (!ToOriginItemWidget) return false;
	ToOriginItemWidget = ToOriginItemWidget->GetOriginGridItemWidget();
	if (ToOriginItemWidget == nullptr)
	{
		return false;
	}

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

	// UE_LOG(LogInventorySystem, Log, TEXT("To = %s"), *ToPosition.ToString());
	// UE_LOG(LogInventorySystem, Log, TEXT("Origin To = %s"), *ToOriginItemWidget->GetPosition().ToString());
	// 收集所有被FromItem覆盖的网格
	TArray<UGridInvSys_ContainerGridItemWidget*> OutWidgets;
	GetOccupiedGridItems(OutWidgets, ToPosition, FromItemSize); // 得到物品想要占据的所有网格控件
	// 限制条件，只有当 From 物品完全包裹 To 的所有物品 或是 To 完全包裹 From的物品 时才能交换物品。
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : OutWidgets) 
	{
		// 遍历判断所有网格控件，获取它的锚点位置，判断是否再覆盖范围内。
		FIntPoint TempOriginPosition = ItemWidget->GetOriginPosition();
		if (!IsInRange(TempOriginPosition, ItemWidget->GetItemSize(), ToPosition, FromItemSize))
		{
			// UE_LOG(LogInventorySystem, Log, TEXT("To物品不在From的范围内: To[%s:%s] <===> From[%s:%s]"),
			// 	*TempOriginPosition.ToString(),*ItemWidget->GetItemSize().ToString(),
			// 	*ToPosition.ToString(),*FromItemSize.ToString())
			if (!IsInRange(ToPosition, FromItemSize, TempOriginPosition, ItemWidget->GetItemSize()))
			{
				// UE_LOG(LogInventorySystem, Log, TEXT("From物品不在To的范围内: To[%s:%s] <===> From[%s:%s]"),
				// 	*TempOriginPosition.ToString(),*ItemWidget->GetItemSize().ToString(),
				// 	*ToPosition.ToString(),*FromItemSize.ToString())
				return false;
			}
		}
		// if (!IsInRange(TempOriginPosition, ItemWidget->GetItemSize(), ToPosition, FromItemSize) &&
		// 	!IsInRange(ToPosition, FromItemSize, TempOriginPosition, ItemWidget->GetItemSize()))
		// {
		// 	UE_LOG(LogInventorySystem, Log, TEXT("有物体在范围外: To[%s:%s] <===> From[%s:%s]"),
		// 		*TempOriginPosition.ToString(),*ItemWidget->GetItemSize().ToString(),
		// 		*ToPosition.ToString(),*FromItemSize.ToString())
		// 	return false;
		// }
	}

	// To 未被占领或是 From 
	if (ToOriginItemWidget->IsOccupied() == false)
	{
		if (HasEnoughFreeSpace(ToPosition, FromItemSize))
		{
			//UE_LOG(LogInventorySystem, Log, TEXT("目标位置未被占领且存在足够的范围"))
			return true;
		}
	}
	else if (FromItemSize == ToOriginItemWidget->GetItemSize() && ToOriginItemWidget == ToOriginItemWidget->GetOriginGridItemWidget())
	{
		// To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角
		//UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角"))
		return true;
	}

	// 物品发生旋转时，范围内不能存在其他物品。
	// if (FromItemSize != FromGridItemWidget->GetItemSize())
	// {
	// 	UE_LOG(LogInventorySystem, Log, TEXT("物品发生旋转时，范围内不能存在其他物品。"))
	// 	return HasEnoughFreeSpace(ToPosition, FromItemSize, FromGridItemWidget->GetOccupiedGridItems());
	// }

	// To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小
	if (FromItemSize.X >= ToOriginItemWidget->GetItemSize().X && FromItemSize.Y >= ToOriginItemWidget->GetItemSize().Y)
	{
		// 注意：前面已经判断过拖拽物品要放置的范围内的所有其他物品都在它的范围内
		// 所以这里判断拖拽物品的大小 >= 目标位置的大小就可以返回 True
		//UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 >= 放置位置的物品的大小"))
		return true;
	}
	else if (FromContainer == this)
	{
		// 如果拖拽物品的大小 < 目标位置的物品大小
		// 需要判断拖拽物品与目标位置的物品是否再同一容器布局内，若不在同一容器则返回False
		FIntPoint Position;
		
		TArray<UWidget*> Ignores = ToOriginItemWidget->GetOccupiedGridItems();
		TArray<UWidget*> NotIgnores;
		GetContainerGridItems(NotIgnores, ToPosition, FromItemSize);
		// 被From占据的网格不会被忽略
		for (UWidget* NotIgnore : NotIgnores)
		{
			Ignores.Remove(NotIgnore);
		}
		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		//UE_LOG(LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小"))
		return FindValidPosition(ToOriginItemWidget->GetItemSize(), Position, Ignores);
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
	if (InOperation->DefaultDragVisual == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}
	// 拖拽目标的物品实例
	UInvSys_InventoryItemInstance* DragItemInstance = IInvSys_DraggingItemInterface::Execute_GetItemInstance(InOperation->DefaultDragVisual);
	check(DragItemInstance);
	if (DragItemInstance)
	{
		const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
		const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
		auto GridItemSizeFragment = DragItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
		auto DragDropFragment = DragItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (GridItemSizeFragment && DragDropFragment)
		{
			FIntPoint NativeItemSize = GridItemSizeFragment->ItemSize;
			FIntPoint NativeOriginPosition = CalculateGridOriginPoint(LocalPosition, NativeItemSize);
			if (NativeOriginPosition == LastDropOriginPosition)
			{
				return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
			}
			// 将上一帧的效果清除。
			ResetDragDropData();
			// 第一次放置判断，根据物品默认的方向判断是否可以放置
			bool bIsCanDrop = IsCanDropItemFromContainer(this, NativeOriginPosition, NativeItemSize);
			if (DragDropFragment->bIsAutoRotation == true && bIsCanDrop == false && NativeItemSize.X != NativeItemSize.Y)
			{
				// 第二次放置判定，切换XY
				FIntPoint RevItemSize = FIntPoint(NativeItemSize.Y, NativeItemSize.X);
				FIntPoint RevOriginPosition = CalculateGridOriginPoint(LocalPosition, RevItemSize);
				if (IsCanDropItemFromContainer(this, RevOriginPosition, RevItemSize))
				{
					IGridInvSys_DraggingItemInterface::Execute_UpdateItemDirection(InOperation->DefaultDragVisual,
						EGridInvSys_ItemDirection::Vertical);
					ShowDragGridEffect(RevOriginPosition, RevItemSize, true);
					return true;
				}
			}
			IGridInvSys_DraggingItemInterface::Execute_UpdateItemDirection(InOperation->DefaultDragVisual,
				EGridInvSys_ItemDirection::Horizontal);
			ShowDragGridEffect(NativeOriginPosition, NativeItemSize, bIsCanDrop);
			return true;
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

	if (InOperation->DefaultDragVisual == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}
	// 拖拽目标的物品实例
	UInvSys_InventoryItemInstance* DragItemInstance = IInvSys_DraggingItemInterface::Execute_GetItemInstance(InOperation->DefaultDragVisual);
	check(DragItemInstance);
	if (DragItemInstance)
	{
		const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
		const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
		auto GridItemSizeFragment = DragItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
		auto DragDropFragment = DragItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (GridItemSizeFragment && DragDropFragment)
		{
			EGridInvSys_ItemDirection ItemDirection = IGridInvSys_DraggingItemInterface::Execute_GetItemDirection(InOperation->DefaultDragVisual);
			FIntPoint NativeItemSize = GridItemSizeFragment->ItemSize;
			FIntPoint ItemSize = NativeItemSize;
			// 根据放下计算实际大小
			switch (ItemDirection)
			{
			case EGridInvSys_ItemDirection::Horizontal:
				ItemSize = NativeItemSize;
				break;
			case EGridInvSys_ItemDirection::Vertical:
				ItemSize = FIntPoint(NativeItemSize.Y, NativeItemSize.X);
				break;
			}
			
			FIntPoint OriginPosition = CalculateGridOriginPoint(LocalPosition, ItemSize);
			if (IsCanDropItemFromContainer(this, OriginPosition, ItemSize))
			{
				FGridInvSys_ItemPosition ToPos;
				ToPos.EquipSlotTag = SlotTag;
				ToPos.GridID = 0;
				ToPos.Position = OriginPosition;
				ToPos.Direction = ItemDirection;
				return TryDropItemFromContainer(this, DragItemInstance, ItemSize, ToPos);
			}
		}
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

bool UGridInvSys_ContainerGridWidget::TryDropItemFromContainer(UGridInvSys_ContainerGridWidget* FromContainer,
	UInvSys_InventoryItemInstance* ItemInstance, FIntPoint FromItemSize, FGridInvSys_ItemPosition ItemPositionData)
{
	UE_LOG(LogInventorySystem, Log, TEXT("尝试放下物品至位置：%s"), *ItemPositionData.ToString())

	UGridInvSys_InventoryComponent* PlayerInvComp = GetOwningPlayer()->GetComponentByClass<UGridInvSys_InventoryComponent>();
	check(PlayerInvComp);
	if (PlayerInvComp == nullptr)
	{
		return false;
	}
	
	UGridInvSys_InventoryItemInstance* From_GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	check(From_GridItemInstance)
	if (From_GridItemInstance == nullptr)
	{
		return false;
	}
	
	FGridInvSys_ItemPosition FromItemPosition = From_GridItemInstance->GetItemPosition();
	
	// 记录修改之后物品的新信息
	TArray<FGridInvSys_ItemPosition> NewItemsData;
	TArray<UInvSys_InventoryItemInstance*> NewItemInstance;
	
	FIntPoint ToPosition = ItemPositionData.Position;


	// IsCanDrop == true 所以可以直接将 From 的位置修改为目标位置并记录下来。
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetGridItemWidget(ToPosition);

	// 获取 From 在容器内占据的所有网格
	// 查询目标位置的范围内所有已经被占据网格，注意：这里会忽略 From 位置下的网格。
	TArray<UGridInvSys_ContainerGridItemWidget*> ToOccupiedItems;
	GetOccupiedGridItems(ToOccupiedItems, ToPosition, FromItemSize);
	if (ToOccupiedItems.IsEmpty()) // 目标位置未被其他物品占领
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] To 未被占领或是 From 与 To 的 Occupant 是同一对象"))
		PlayerInvComp->Server_TryDropItemInstanceToPos(InventoryComponent.Get(), ItemInstance, ItemPositionData);
		return true;
	}
	
#if 0
	// To 的位置已经被其他物品占领，但 From 与 To 的大小一致且 To 的位置是物品的左上角
	if (FromItemSize == ToGridItemWidget->GetItemSize() &&
		ToGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[Try Drop Item] From 与 To 的大小一致且 To 的位置是物品的左上角"))
		// 交换双方位置
		//PlayerInvComp->Server_AddItemInstanceToContainerPos(); // 让目标位置的物品转移到FromContainer
		//PlayerInvComp->
		UInvSys_InventoryItemInstance* ToItemInstance = ToGridItemWidget->GetItemInstance();
		
		//PlayerInvComp->Server_UpdateItemInstancePos(InventoryComponent, FromInvComp, ToItemInstance, FromItemPosition);
		// From, To, ItemInstance, Position
		// PlayerInvComp->Server_AddItemInstanceToContainerPos(InventoryComponent.Get(), ItemInstance, ItemPositionData);
		return true;
	}

	// 不同容器下的物品交换
	UGridInvSys_ContainerGridWidget* FromContainerGrid = FromContainer->GetContainerGridWidget();
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
				FIntPoint TargetPosition = FromContainer->GetOriginPosition();
				TargetPosition.X += RelativePosition.X;
				TargetPosition.Y += RelativePosition.Y;
				// UE_LOG(LogInventorySystem, Log, TEXT("计算得到的最终坐标为 [%d,%d]"), TargetPosition.X, TargetPosition.Y);
				// 计算这些ToOccupiedItems在From容器下的位置
				FGridInvSys_InventoryItemPosition TempItemData;
				TempItemData.Position = TargetPosition;
				TempItemData.SlotName = FromContainerGrid->GetSlotName();
				TempItemData.GridID = NAME_None;
				TempItemData.Direction = ToOccupiedItem->GetItemDirection(); // 方向保持不变。
				NewItemsData.Add(TempItemData);
				ChangedItems.Add(ToOccupiedItem->GetItemUniqueID());
			}
			PlayerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
			return true;
		}
	}

	// To 的位置已经被其他物品占领，且拖拽的物品大小 > 放置位置的物品的大小
	// 注意：
	if (FromItemSize.X >= ToGridItemWidget->GetItemSize().X &&
		FromItemSize.Y >= ToGridItemWidget->GetItemSize().Y ||
		FromContainer->GetOriginGridItemWidget() == ToGridItemWidget->GetOriginGridItemWidget())
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
			FIntPoint RelativePosition = ToOccupiedItem->CalculateRelativePosition(GetGridItemWidget(ToPosition));
			
			// 转换为对角坐标
			FIntPoint DiagonalPosition = FromContainer->GetOriginPosition();
			DiagonalPosition.X += FromContainer->GetItemSize().X - RelativePosition.X - ToOccupiedItem->GetItemSize().X;
			DiagonalPosition.Y += FromContainer->GetItemSize().Y - RelativePosition.Y - ToOccupiedItem->GetItemSize().Y;
			UE_LOG(LogInventorySystem, Log, TEXT("对角坐标.X = %d + %d - %d - %d"),FromContainer->GetOriginPosition().X,
				FromItemSize.X, RelativePosition.X, ToOccupiedItem->GetItemSize().X)
			UE_LOG(LogInventorySystem, Log, TEXT("对角坐标.Y = %d + %d - %d - %d"), FromContainer->GetOriginPosition().Y,
				FromItemSize.Y, RelativePosition.Y, ToOccupiedItem->GetItemSize().Y)

			// 计算这些ToOccupiedItems在From容器下的位置
			FGridInvSys_InventoryItemPosition TempItemData;
			TempItemData.Position = DiagonalPosition;
			TempItemData.SlotName = FromContainerGrid->GetSlotName();
			TempItemData.GridID = NAME_None;
			TempItemData.Direction = ToOccupiedItem->GetItemDirection(); // 方向保持不变。
			NewItemsData.Add(TempItemData);
			ChangedItems.Add(ToOccupiedItem->GetItemUniqueID());
		}
		PlayerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
		return true;
	}
	else
	{
		// from size 小于 to size
		FIntPoint Position;
		TArray<UWidget*> Ignores = FromContainer->GetOccupiedGridItems();
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
			TempItemData.GridID = NAME_None;
			TempItemData.Direction = ToGridItemWidget->GetItemDirection(); // 方向保持不变。
			NewItemsData.Add(TempItemData);
			ChangedItems.Add(ToGridItemWidget->GetItemUniqueID());
			PlayerInvComp->Server_UpdateInventoryItems(GetInventoryComponent(), ChangedItems, NewItemsData);
			return true;
		}
	}
#endif
	return false;
}

void UGridInvSys_ContainerGridWidget::ShowDragGridEffect(FIntPoint Position, FIntPoint Size, bool bIsRight)
{
	GetContainerGridDragItems<UGridInvSys_ContainerGridDropWidget>(LastDropOverItems, Position, Size);
	for (UGridInvSys_ContainerGridDropWidget* DropOverItem : LastDropOverItems)
	{
		DropOverItem->NativeOnDraggingHovered(bIsRight);
	}
}

void UGridInvSys_ContainerGridWidget::UpdateInventoryItem(const FGridInvSys_InventoryItem& InventoryItem)
{
	const FIntPoint ToPosition = InventoryItem.ItemPosition.Position; 
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetGridItemWidget(ToPosition);
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
	UGridInvSys_ContainerGridItemWidget* RemoveGridItemWidget = GetGridItemWidget(ToPosition);
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
		ItemWidget->RemoveItemInstance();
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
			int32 Index = GetItemIndex(FIntPoint(Position.X + i, Position.Y + j));
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
