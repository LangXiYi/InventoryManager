// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
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
			UGridInvSys_ContainerGridItemWidget* GridItemWidget = GetGridItemWidget(FIntPoint(Position.X + i, Position.Y + j));
			// 仅处理 UContainerGridItemWidget 及其子类。
			if (GridItemWidget != nullptr && Ignores.Contains(GridItemWidget) == false)
			{
				OutArray.AddUnique(GridItemWidget);
			}
		}
	}
}

void UGridInvSys_ContainerGridWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (GridItemWidgetClass == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("GridItemWidgetClass is nullptr."))
		return;
	}

	//  清除 ContainerGrid 中所有子项。
	if (ContainerPanel->HasAnyChildren() == true)
	{
		ContainerPanel->ClearChildren();
	}

	// 创建所有网格
	int32 ItemSize = GetDefault<UGridInvSys_InventorySystemConfig>()->ItemDrawSize;
	for (int i = 0; i < ContainerGridSize.X * ContainerGridSize.Y; ++i)
	{
		const int SlotRow = i / ContainerGridSize.Y;
		const int SlotColumn = i % ContainerGridSize.Y;
		auto GridItemWidget = CreateWidget<UGridInvSys_ContainerGridItemWidget>(this, GridItemWidgetClass);
		UCanvasPanelSlot* PanelSlot = ContainerPanel->AddChildToCanvas(GridItemWidget);

		PanelSlot->SetPosition(FVector2d(SlotColumn * ItemSize, SlotRow * ItemSize));
		PanelSlot->SetSize(FVector2d(ItemSize, ItemSize)); // 方便后续显示拖拽范围。

		// GridItemWidget->SetInventoryObject(GetInventoryObject());
		// GridItemWidget->SetInventoryComponent(InventoryComponent.Get());
		// GridItemWidget->SetSlotTag(SlotTag);
		GridItemWidget->OnConstructGridItem(this, FIntPoint(SlotRow, SlotColumn));
	}
}

int32 UGridInvSys_ContainerGridWidget::GetItemIndex(const FIntPoint Position) const
{
	return IsValidPosition(Position) ? Position.X * ContainerGridSize.Y + Position.Y : -1;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridWidget::GetGridItemWidget(FIntPoint Position) const
{
	const int32 Index = GetItemIndex(Position);
	return Index >= 0 ? Cast<UGridInvSys_ContainerGridItemWidget>(ContainerPanel->GetChildAt(Index)) : nullptr;
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

bool UGridInvSys_ContainerGridWidget::IsCanDropItemFromContainer(
	UInvSys_InventoryItemInstance* ItemInstance, FIntPoint ToPosition, EGridInvSys_ItemDirection ItemDirection) const
{
#pragma region 常规检查
	check(ItemInstance)
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	if (GridItemInstance == nullptr)
	{
		checkNoEntry()
		return false;
	}
	UGridInvSys_InventoryComponent* GridInvComp = GridItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
	if (GridInvComp == nullptr)
	{
		checkNoEntry()
		return false;
	}

	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetGridItemWidget(ToPosition);
	if (ToGridItemWidget == nullptr)
	{
		return false;
	}
	UGridInvSys_InventoryItemInstance* ToGridItemInstance = ToGridItemWidget->GetItemInstance<UGridInvSys_InventoryItemInstance>();

#pragma endregion

	/***
	 * 【【【 前置条件 】】】
	 * 1、物品在容器范围内
	 * 2、范围内的所有物品都在物品实例的覆盖范围下
	 */
	FIntPoint TargetItemSize = GridItemInstance->GetItemSize(ItemDirection);
	if (!IsInContainer(ToPosition, TargetItemSize))
	{
		return false;
	}
	TArray<UGridInvSys_InventoryItemInstance*> OutItemInstances;
	GetAllHoveredItemInstances(OutItemInstances, ToPosition, TargetItemSize);
	for (UGridInvSys_InventoryItemInstance* HoveredItemInstance : OutItemInstances) 
	{
		auto ItemSizeFragment = HoveredItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
		if (ItemSizeFragment == nullptr)
		{
			return false;
		}
		// 计算被覆盖的物品大小
		FGridInvSys_ItemPosition HoveredItemPosition = HoveredItemInstance->GetItemPosition();
		FIntPoint HoveredItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(HoveredItemInstance);
		if (!IsInRange(HoveredItemPosition.Position, HoveredItemSize, ToPosition, TargetItemSize))
		{
			if (!IsInRange(ToPosition, TargetItemSize, HoveredItemPosition.Position, HoveredItemSize))
			{
				// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("From物品不在To的范围内 & To 也不在 From 的范围内"))
				return false;
			}
		}
	}
	OutItemInstances.Empty();

	/***
	 * 目标位置未被其他物品占据 & 足够容纳物品实例
	 */

	if (ToGridItemWidget->IsOccupied() == false)
	{
		if (HasEnoughFreeSpace(ToPosition, TargetItemSize))
		{
			// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("目标位置未被占领且存在足够的范围"))
			return true;
		}
	}
	
	/***
	 * 目标位置下的物品与传入的物品实例大小一致 & 目标位置与重叠物品的位置一致
	 */

	FIntPoint ToTargetItemSize = ToGridItemInstance->GetItemSize();
	if (TargetItemSize == ToTargetItemSize && ToGridItemWidget == ToGridItemWidget->GetOriginGridItemWidget())
	{
		// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，但 From 与 To 的大小一致 且 To 的位置是物品的左上角"))
		return true;
	}

	/***
	 * 物品实例不在当前容器内 & 物品实例的大小 >= 占据目标位置的物品的大小
	 */

	if (GridInvComp != this->GetInventoryComponent() || GridItemInstance->GetSlotTag() != this->GetSlotTag())
	{
		if (TargetItemSize.X >= ToTargetItemSize.X &&
			TargetItemSize.Y >= ToTargetItemSize.Y)
		{
			// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 >= 放置位置的物品的大小"))
			return true;
		}
	}
	
	/***
	 * A: 物品实例在当前容器内 & 物品实例的大小 >= 占据目标位置的物品的大小 & 转移的位置不在物品实例的覆盖范围内
	 * B: 物品实例在当前容器内 & 物品实例的大小 < 占据目标位置的物品的大小
	 */

	if (TargetItemSize.X >= ToTargetItemSize.X && TargetItemSize.Y >= ToTargetItemSize.Y)
	{
		// TODO::转移的位置不在物品实例的覆盖范围内
		TArray<UGridInvSys_InventoryItemInstance*> AllOccupiedObjects;
		GetAllHoveredItemInstances(AllOccupiedObjects, ToPosition, TargetItemSize);
		for (UGridInvSys_InventoryItemInstance* OccupiedItemInstance : AllOccupiedObjects)
		{
			// 计算相对坐标
			FGridInvSys_ItemPosition TempItemPos = OccupiedItemInstance->GetItemPosition();
			FGridInvSys_ItemPosition FromItemPosition = GridItemInstance->GetItemPosition();
			FIntPoint RelativePosition;
			RelativePosition.X = TempItemPos.Position.X - ToPosition.X;
			RelativePosition.Y = TempItemPos.Position.Y - ToPosition.Y;
			// 如果物品原始的方向与当前方向不同，则需要对物品位置进行旋转
			if (FromItemPosition.Direction != ItemDirection)
			{
				int32 TempPos = RelativePosition.X;
				RelativePosition.X = RelativePosition.Y;
				RelativePosition.Y = TempPos;
			}

			// 计算被覆盖的物品实例的大小 & 拖拽前目标物品的大小
			FIntPoint TargetOccupiedItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(OccupiedItemInstance);
			FIntPoint PreDragItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(GridItemInstance, FromItemPosition.Direction);	
			// 转换为对角坐标
			FIntPoint DiagonalPosition;
			DiagonalPosition.X = (PreDragItemSize.X - RelativePosition.X - TargetOccupiedItemSize.X);
			DiagonalPosition.Y = (PreDragItemSize.Y - RelativePosition.Y - TargetOccupiedItemSize.Y);
			DiagonalPosition.X = DiagonalPosition.X + FromItemPosition.Position.X;
			DiagonalPosition.Y = DiagonalPosition.Y + FromItemPosition.Position.Y;

			// 如果这个物品的新位置处于 ItemPositionData.Position + TargetItemSize 的范围内，则返回False
			if (IsInRange(DiagonalPosition, TargetOccupiedItemSize, ToPosition, TargetItemSize))
			{
				return false;
			}
		}
		// UE_LOG(LogInventorySystem, Log, TEXT("可以放置 Item "))
		return true;
	}
	else
	{
#if 1
		// todo::不是很推荐使用小物品拖拽替换大物品
		// 如果拖拽物品的大小 < 目标位置的物品大小
		// 需要判断拖拽物品与目标位置的物品是否再同一容器布局内，若不在同一容器则返回False
		FIntPoint Position;
		
		TArray<UWidget*> Ignores = ToGridItemWidget->GetOccupiedGridItems();
		TArray<UWidget*> NotIgnores;
		GetContainerGridItems(NotIgnores, ToPosition, TargetItemSize);
		// 被From占据的网格不会被忽略
		for (UWidget* NotIgnore : NotIgnores)
		{
			Ignores.Remove(NotIgnore);
		}
		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小"))
		return FindValidPosition(ToTargetItemSize, Position, Ignores);
#endif
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
	if (InOperation->DefaultDragVisual == nullptr || InOperation->Payload == nullptr)
	{
		return false;
	}
	// 拖拽目标的物品实例
	UGridInvSys_InventoryItemInstance* DragItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InOperation->Payload);
	if (DragItemInstance == nullptr)
	{
		return false;
	}
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
			return false;
		}
		// 将上一帧的效果清除。
		ResetDragDropData();
		// 第一次放置判断，根据物品默认的方向判断是否可以放置
		bool bIsCanDrop = IsCanDropItemFromContainer(DragItemInstance, NativeOriginPosition, EGridInvSys_ItemDirection::Horizontal);
		if (DragDropFragment->bIsAutoRotation == true && bIsCanDrop == false && NativeItemSize.X != NativeItemSize.Y)
		{
			// 第二次放置判定，切换XY
			FIntPoint RevItemSize = FIntPoint(NativeItemSize.Y, NativeItemSize.X);
			FIntPoint RevOriginPosition = CalculateGridOriginPoint(LocalPosition, RevItemSize);
			if (IsCanDropItemFromContainer(DragItemInstance, RevOriginPosition, EGridInvSys_ItemDirection::Vertical))
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
		return false;
	}
	// 拖拽目标的物品实例
	UGridInvSys_InventoryItemInstance* DragItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InOperation->Payload);
	if (DragItemInstance == nullptr	)
	{
		return false;
	}
	
	const FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
	
	auto DragDropFragment = DragItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment)
	{
		EGridInvSys_ItemDirection ItemDirection =
			IGridInvSys_DraggingItemInterface::Execute_GetItemDirection(InOperation->DefaultDragVisual);

		FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(DragItemInstance, ItemDirection);;
		// 根据屏幕坐标以及物品的大小计算最终的槽位坐标
		// TODO::这个函数应该只支持 Center Center 格式的 Drag 对于其他格式计算可能出现错误
		FIntPoint TargetSlotPosition = CalculateGridOriginPoint(LocalPosition, ItemSize);
		
		if (IsCanDropItemFromContainer(DragItemInstance, TargetSlotPosition, ItemDirection))
		{
			FGridInvSys_ItemPosition DropPosition;
			DropPosition.EquipSlotTag = SlotTag;
			DropPosition.GridID = ContainerGridID;
			DropPosition.Position = TargetSlotPosition;
			DropPosition.Direction = ItemDirection;
			return TryDropItemFromContainer(DragItemInstance, DropPosition);
		}
	}
	return false;
}

void UGridInvSys_ContainerGridWidget::ResetDragDropData()
{
	for (UGridInvSys_ContainerGridItemWidget* LastOverItem : LastDropOverItems)
	{
		LastOverItem->OnEndDraggingHovered();
	}
	LastDropOverItems.Empty();
	LastDropOriginPosition = FIntPoint(-1, -1);
}

// 优化传入参数：ItemInstance、FromContainer、ToPosition
bool UGridInvSys_ContainerGridWidget::TryDropItemFromContainer(
	UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition DropPosition)
{
	UGridInvSys_GridInventoryControllerComponent* PlayerInvComp =
		UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent<UGridInvSys_GridInventoryControllerComponent>(GetWorld());
	if (PlayerInvComp == nullptr)
	{
		return false;
	}
	
	check(ItemInstance)
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	if (GridItemInstance == nullptr)
	{
		checkNoEntry()
		return false;
	}
	UGridInvSys_InventoryComponent* GridInvComp = GridItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
	if (GridInvComp == nullptr)
	{
		checkNoEntry()
		return false;
	}

	FIntPoint FromTargetItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(ItemInstance, DropPosition.Direction);
	
	// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("尝试放下物品至位置：%s"), *DropPosition.ToString())

	// IsCanDrop == true 所以可以直接将 From 的位置修改为目标位置并记录下来。
	UGridInvSys_ContainerGridItemWidget* ToGridItemWidget = GetGridItemWidget(DropPosition.Position);
	UGridInvSys_InventoryItemInstance* ToGridItemInstance = ToGridItemWidget->GetItemInstance<UGridInvSys_InventoryItemInstance>();

	// 获取 From 在容器内占据的所有网格
	// 目标位置未被其他物品占领
	if (ToGridItemInstance == nullptr && HasEnoughFreeSpace(DropPosition.Position, FromTargetItemSize)) 
	{
		PlayerInvComp->Server_TryDropItemInstance(InventoryComponent.Get(), ItemInstance, DropPosition);
		return true;
	}

	FIntPoint ToTargetItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(ToGridItemInstance);
	// To 的位置已经被其他物品占领，但 From 与 To 的大小一致且 To 的位置是物品的左上角
	if (FromTargetItemSize == ToTargetItemSize &&
		ToGridItemWidget->GetOriginGridItemWidget() == ToGridItemWidget)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[Try Drop Item] From 与 To 的大小一致且 To 的位置是物品的左上角"))
		// 交换双方位置
		UInvSys_InventoryItemInstance* ToItemInstance = ToGridItemWidget->GetItemInstance();
		check(ToItemInstance)
		PlayerInvComp->Server_SwapItemInstance(ItemInstance, ToItemInstance);
		return true;
	}

	/***
	 * 物品实例不在当前容器内 & 物品实例的大小 >= 占据目标位置的物品的大小
	 */

	// 不同容器下的物品交换 且  拖拽的物品大小 >= 放置位置的物品的大小
	if (GridInvComp != this->GetInventoryComponent() || GridItemInstance->GetSlotTag() != this->GetSlotTag())
	{
		if (FromTargetItemSize.X >= ToTargetItemSize.X &&
			FromTargetItemSize.Y >= ToTargetItemSize.Y)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[Try Drop Item] 不同容器下的物品交换To 位置可以容纳 From 对象"))
			TArray<UGridInvSys_InventoryItemInstance*> AllHoveredItemInstances;
			GetAllHoveredItemInstances(AllHoveredItemInstances, DropPosition.Position, FromTargetItemSize);
			for (UGridInvSys_InventoryItemInstance* HoveredItemInstance : AllHoveredItemInstances)
			{
				FGridInvSys_ItemPosition TempItemPos = HoveredItemInstance->GetItemPosition();
				FGridInvSys_ItemPosition FromItemPosition = GridItemInstance->GetItemPosition();
				// ToPosition TempPos.Pos 计算相对坐标
				FIntPoint RelativePosition;
				RelativePosition.X = TempItemPos.Position.X - DropPosition.Position.X;
				RelativePosition.Y = TempItemPos.Position.Y - DropPosition.Position.Y;
				// 如果物品原始的方向与当前方向不同，则需要对物品位置进行旋转
				if (FromItemPosition.Direction != DropPosition.Direction)
				{
					int32 TempPos = RelativePosition.X;
					RelativePosition.X = RelativePosition.Y;
					RelativePosition.Y = TempPos;
				}
				// 计算最终在From容器中的坐标
				FIntPoint TargetPosition = FromItemPosition.Position;
				TargetPosition.X += RelativePosition.X;
				TargetPosition.Y += RelativePosition.Y;
	
				// 更新数据
				TempItemPos.Position = TargetPosition;
				TempItemPos.EquipSlotTag = FromItemPosition.EquipSlotTag;
				TempItemPos.GridID = FromItemPosition.GridID;
				// TempItemPos.Direction; // 方向保持不变。
				PlayerInvComp->Server_UpdateItemInstancePosition(ItemInstance->GetInventoryComponent(), HoveredItemInstance, TempItemPos);
			}
			PlayerInvComp->Server_TryDropItemInstance(InventoryComponent.Get(), ItemInstance, DropPosition);
			return true;
		}
		// return false;
	}
	// 相同容器下物品交换 且 拖拽的物品大小 >= 放置位置的物品的大小
	if (FromTargetItemSize.X >= ToTargetItemSize.X && FromTargetItemSize.Y >= ToTargetItemSize.Y)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("[Try Drop Item] 相同容器下交换，To 的位置已经被其他物品占领，且拖拽的物品大小 >= 放置位置的物品的大小"))
		TArray<UGridInvSys_InventoryItemInstance*> AllHoveredItemInstances;
		GetAllHoveredItemInstances(AllHoveredItemInstances, DropPosition.Position, FromTargetItemSize);

		TArray<FGridInvSys_ItemPosition> NewItemPositions;
		for (UGridInvSys_InventoryItemInstance* HoveredItemInstance : AllHoveredItemInstances)
		{
			// 计算相对坐标
			FGridInvSys_ItemPosition TempItemPos = HoveredItemInstance->GetItemPosition();
			FGridInvSys_ItemPosition FromItemPosition = GridItemInstance->GetItemPosition();
			FIntPoint RelativePosition;
			RelativePosition.X = TempItemPos.Position.X - DropPosition.Position.X;
			RelativePosition.Y = TempItemPos.Position.Y - DropPosition.Position.Y;
			// 如果物品原始的方向与当前方向不同，则需要对物品位置进行旋转
			if (FromItemPosition.Direction != DropPosition.Direction)
			{
				int32 TempPos = RelativePosition.X;
				RelativePosition.X = RelativePosition.Y;
				RelativePosition.Y = TempPos;
			}

			// 计算被覆盖的物品实例的大小 & 拖拽前目标物品的大小
			FIntPoint TargetOccupiedItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(HoveredItemInstance);
			FIntPoint PreDragItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(GridItemInstance, FromItemPosition.Direction);	
			// 转换为对角坐标
			FIntPoint DiagonalPosition;
			DiagonalPosition.X = (PreDragItemSize.X - RelativePosition.X - TargetOccupiedItemSize.X);
			DiagonalPosition.Y = (PreDragItemSize.Y - RelativePosition.Y - TargetOccupiedItemSize.Y);
			DiagonalPosition.X = DiagonalPosition.X + FromItemPosition.Position.X;
			DiagonalPosition.Y = DiagonalPosition.Y + FromItemPosition.Position.Y;

			// 如果这个物品的新位置处于 ItemPositionData.Position + TargetItemSize 的范围内，则返回False
			if (IsInRange(DiagonalPosition, TargetOccupiedItemSize, DropPosition.Position, FromTargetItemSize))
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("交换位置失败，拖拽的物品与目标物品会发生重叠！！"))
				return false;
			}
			// 更新数据
			TempItemPos.Position = DiagonalPosition;
			TempItemPos.EquipSlotTag = FromItemPosition.EquipSlotTag;
			TempItemPos.GridID = FromItemPosition.GridID;
			// TempItemPos.Direction; // 方向保持不变。
			NewItemPositions.Add(TempItemPos);
		}
		for (int i = 0; i < AllHoveredItemInstances.Num(); ++i)
		{
			PlayerInvComp->Server_UpdateItemInstancePosition(
				InventoryComponent.Get(), AllHoveredItemInstances[i], NewItemPositions[i]);
		}
		PlayerInvComp->Server_TryDropItemInstance(InventoryComponent.Get(), ItemInstance, DropPosition);
		return true;
	}
	else // from size 小于 to size & 可能在同一容器或不同容器
	{
		// 获得需要被忽略的网格
		TArray<UWidget*> Ignores = ToGridItemWidget->GetOccupiedGridItems();
		TArray<UWidget*> NotIgnores;
		GetContainerGridItems(NotIgnores, DropPosition.Position, FromTargetItemSize);
		GetContainerGridItems(Ignores, ToGridItemWidget->GetOriginPosition(), ToTargetItemSize, NotIgnores);
		NotIgnores.Empty();

		// 在容器内查找其他可以放置该物品的位置，From占据的网格会被视为空闲。
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[%d] To 的位置已经被其他物品占领，且拖拽的物品大小 < 放置位置的物品的大小"), Ignores.Num())
		// 查找 To 物品 可以放置的位置
		FIntPoint ValidPosition;
		if (FindValidPosition(ToTargetItemSize, ValidPosition, Ignores))
		{
			// 计算 To 物品在From容器下的位置
			FGridInvSys_ItemPosition TempItemData = ToGridItemInstance->GetItemPosition();
			TempItemData.Position = ValidPosition;
			PlayerInvComp->Server_UpdateItemInstancePosition(GetInventoryComponent(), ToGridItemInstance, TempItemData);
			PlayerInvComp->Server_TryDropItemInstance(GetInventoryComponent(), ItemInstance, DropPosition);
			return true;
		}
	}
	return false;
}

void UGridInvSys_ContainerGridWidget::ShowDragGridEffect(FIntPoint Position, FIntPoint Size, bool bIsRight)
{
	// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("Pos = [%d, %d]"), Position.X, Position.Y)
	GetContainerGridItems<UGridInvSys_ContainerGridItemWidget>(LastDropOverItems, Position, Size);
	for (UGridInvSys_ContainerGridItemWidget* DropOverItem : LastDropOverItems)
	{
		DropOverItem->OnDraggingHovered(bIsRight);
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
	for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : OutArray)
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

void UGridInvSys_ContainerGridWidget::GetAllHoveredItemInstances(TArray<UGridInvSys_InventoryItemInstance*>& OutArray,
	FIntPoint Position, FIntPoint Size) const
{
	TArray<UGridInvSys_ContainerGridItemWidget*> TempArray;
	GetContainerGridItems(TempArray, Position, Size);
	for (UGridInvSys_ContainerGridItemWidget* ItemWidget : TempArray)
	{
		if (ItemWidget->IsOccupied())
		{
			OutArray.AddUnique(ItemWidget->GetItemInstance<UGridInvSys_InventoryItemInstance>());
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
