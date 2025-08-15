// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/GridInvSys_InventoryModule_Container.h"

#include "GridInvSys_CommonType.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "BaseInventorySystem.h"
#include "GridInventorySystem.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Display.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Equipment.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"

void UGridInvSys_InventoryModule_Container::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);

	COPY_INVENTORY_FRAGMENT_PROPERTY(UGridInvSys_InventoryModule_Container, ContainerLayoutDataType);

	switch (ContainerLayoutDataType)
	{
	case EGridInvSys_ContainerLayoutDataType::FromEquipment:
		UpdateContainerData_FromEquip();
		break;
	case EGridInvSys_ContainerLayoutDataType::FromWidget:
		UpdateContainerData_FromWidget();
		break;
	case EGridInvSys_ContainerLayoutDataType::Custom:
		UpdateContainerData_FromCustom();
		break;
	}

	auto WarpItemPositionFunc = [this](const FGameplayTag& Tag, const FGridInvSys_ItemPositionChangeMessage& Message)
	{
		if (Message.InventoryComponent == GetInventoryComponent() &&
			Message.ItemInstance && Message.ItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
		{
			if (Message.OldPosition.IsValid() && Message.OldPosition.EquipSlotTag == GetInventoryTag())
			{
				UpdateContainerGridItemState((UGridInvSys_InventoryItemInstance*)Message.ItemInstance, Message.OldPosition, false);
			}
			if (Message.NewPosition.IsValid() && Message.NewPosition.EquipSlotTag == GetInventoryTag())
			{
				UpdateContainerGridItemState((UGridInvSys_InventoryItemInstance*)Message.ItemInstance, Message.NewPosition, true);
			}
#if WITH_EDITOR && 0
			if (Message.NewPosition.EquipSlotTag == GetInventoryObjectTag() ||
				Message.OldPosition.EquipSlotTag == GetInventoryObjectTag())
			{
				PrintDebugOccupiedGrid("Listen Item Position Changed");
			}
#endif
		}
	};

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	ItemPositionChangeHandle = GameplayMessageSubsystem.RegisterListener<FGridInvSys_ItemPositionChangeMessage>(Inventory_Message_ItemPositionChanged, WarpItemPositionFunc);
}

bool UGridInvSys_InventoryModule_Container::UpdateItemInstancePosition(
	UGridInvSys_InventoryItemInstance* GridItemInstance, const FGridInvSys_ItemPosition& NewPosition)
{
	bool bIsSuccess = false;
	if (GridItemInstance)
	{
		FIntPoint Size = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(GridItemInstance, NewPosition.Direction);
		UpdateContainerGridItemState(GridItemInstance, GridItemInstance->GetItemPosition(), false); // 清除其在原本位置的占据状态！
		if (IsUnoccupiedInSquareRange(NewPosition.GridID, NewPosition.Position, Size))
		{
			bIsSuccess = true;
			GridItemInstance->SetItemPosition(NewPosition);
			UpdateContainerGridItemState(GridItemInstance, NewPosition, true);
			MarkItemInstanceDirty(GridItemInstance);
		}
	}
	if (bIsSuccess == false)
	{
		UpdateContainerGridItemState(GridItemInstance, GridItemInstance->GetItemPosition(), true);  // 修改位置失败，回退占据状态
	}
	return bIsSuccess;
}

bool UGridInvSys_InventoryModule_Container::IsUnoccupiedInSquareRange(
	int32 ToGridID, FIntPoint ToPosition, FIntPoint ItemSize)
{
	if (ContainerGridSize.IsEmpty())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ContainerGridSize is empty."), __FUNCTION__)
		return false;
	}
	if (OccupiedGrid.IsEmpty())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, OccupiedGrid is empty."), __FUNCTION__)
		return false;
	}
	int32 ToIndex = ToPosition.X * ContainerGridSize[ToGridID].Y + ToPosition.Y;
	int32 MaxGridWidth = ContainerGridSize[ToGridID].Y - 1;
	int32 MaxGridHeight = ContainerGridSize[ToGridID].X - 1;
	for (int X = 0; X < ItemSize.X; ++X)
	{
		for (int Y = 0; Y < ItemSize.Y; ++Y)
		{
			if (ToPosition.X < 0 || ToPosition.Y < 0 ||
				ToPosition.X + X > MaxGridHeight || ToPosition.Y + Y > MaxGridWidth)
			{
				// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error,
				// 	TEXT("[ToPosition.X + X](%d) > [MaxGridWidth](%d) || [ToPosition.Y + Y](%d) > [MaxGridHeight](%d)"),
				// 	ToPosition.X + X, MaxGridWidth, ToPosition.Y + Y, MaxGridHeight)
				return false;
			}
			int32 Index = ToIndex + X * ContainerGridSize[ToGridID].Y + Y;
			if (OccupiedGrid[ToGridID].IsValidIndex(Index) == false ||
				OccupiedGrid[ToGridID][Index] == true)
			{
#if 0
				PrintDebugOccupiedGrid(GetInventoryTag().ToString() + "_" + FString::FromInt(ToGridID) + "--{"
					+ ToPosition.ToString() + "}:::Can't Contains Item For Size [" + ItemSize.ToString() +"]");
#endif
				return false;
			}
		}
	}
	return true;
}

TArray<int32> UGridInvSys_InventoryModule_Container::GetItemGridOccupiedIndexes(
	UGridInvSys_InventoryItemInstance* ItemInstance) const
{
	TArray<int32> OutArray;
	if (ItemInstance)
	{
		if (ItemInstance->GetInventoryComponent() == GetInventoryComponent() && ItemInstance->GetInventoryObjectTag() == InventoryTag)
		{
			if (ContainsItem(ItemInstance))
			{
				FGridInvSys_ItemPosition ItemPosition = ItemInstance->GetItemPosition();

				FIntPoint ItemSize = ItemInstance->GetItemSize();
				FIntPoint ContainerSize = ContainerGridSize[ItemPosition.GridID];
				FIntPoint Position = ItemPosition.Position;
				OutArray.Reset(ItemSize.X * ItemSize.Y);
				for (int i = 0; i < ItemSize.X ; ++i)
				{
					for (int j = 0; j < ItemSize.Y; ++j)
					{
						int32 Index = (Position.X + i) * ContainerSize.Y + Position.Y + j;
						if (OccupiedGrid[ItemPosition.GridID].IsValidIndex(Index))
						{
							OutArray.Add(Index);
						}
					}
				}
			}
		}
	}
	return OutArray;
}

bool UGridInvSys_InventoryModule_Container::CheckItemPosition(
	UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& NewPosition, bool bIsIgnoreInItemInstance)
{
	if (ItemInstance == nullptr || ItemInstance->IsA<UGridInvSys_InventoryItemInstance>() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例为空或类型不为 UGridInvSys_InventoryItemInstance."), __FUNCTION__)
		return false;
	}
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	check(GridItemInstance)
	TArray<int32> ItemGridOccupiedIndexes = {};
	if (bIsIgnoreInItemInstance)
	{
		ItemGridOccupiedIndexes = GetItemGridOccupiedIndexes(GridItemInstance);
	}

	FIntPoint ItemSize = GridItemInstance->GetItemSize(NewPosition.Direction);
	int32 StartIndex = NewPosition.Position.X * ContainerGridSize[NewPosition.GridID].Y + NewPosition.Position.Y;
	FIntPoint ContainerSize = ContainerGridSize[NewPosition.GridID];
	// 循环最大执行数量 = ItemSize.X * ItemSize.Y
	for (int X = 0; X < ItemSize.X; ++X)
	{
		for (int Y = 0; Y < ItemSize.Y; ++Y)
		{
			if (NewPosition.Position.X < 0 || NewPosition.Position.Y < 0 ||
				NewPosition.Position.X + X >= ContainerSize.X || NewPosition.Position.Y + Y >= ContainerSize.Y)
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error,
					TEXT("[ToPosition.X + X](%d) > [MaxGridWidth](%d) || [ToPosition.Y + Y](%d) > [MaxGridHeight](%d)"),
					NewPosition.Position.X + X, ContainerSize.Y, NewPosition.Position.Y + Y, ContainerSize.X)
				return false;
			}
			int32 Index = StartIndex + X * ContainerGridSize[NewPosition.GridID].Y + Y;
			if (OccupiedGrid[NewPosition.GridID].IsValidIndex(Index) == false ||
				OccupiedGrid[NewPosition.GridID][Index] == true)
			{
				// 范围内存在其他被占据的物品，直接返回 false
				if (bIsIgnoreInItemInstance == false)
				{
					return false;
				}
				// 判断位置是否在 ItemInstance 下
				if (ItemGridOccupiedIndexes.Contains(Index) == false)
				{
					PrintDebugOccupiedGrid(GetInventoryTag().ToString() + "_" + FString::FromInt(NewPosition.GridID) + "--{"
						+ NewPosition.Position.ToString() + "}:::Can't Contains Item For Size [" + ItemSize.ToString() +"] ");
					return false;
				}
			}
		}
	}
	return true;
}

void UGridInvSys_InventoryModule_Container::UpdateContainerGridItemState(
	UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& ItemPosition, bool IsOccupy)
{
	if (ItemInstance == nullptr || ItemInstance->IsA<UGridInvSys_InventoryItemInstance>() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr"), __FUNCTION__)
		return;
	}
	auto ItemSizeFragment = ItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
	if (ItemInstance && ItemSizeFragment)
	{
		FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(ItemInstance, ItemPosition.Direction);
		if (OccupiedGrid.IsValidIndex(ItemPosition.GridID) && ContainerGridSize.IsValidIndex(ItemPosition.GridID))
		{
			FIntPoint GridSize = ContainerGridSize[ItemPosition.GridID];
			int32 ItemMax_X = ItemPosition.Position.X + ItemSize.X;
			int32 ItemMax_Y = ItemPosition.Position.Y + ItemSize.Y;
			for (int X = ItemPosition.Position.X; X < ItemMax_X; ++X)
			{
				for (int Y = ItemPosition.Position.Y; Y < ItemMax_Y; ++Y)
				{
					int32 Index = X * GridSize.Y + Y;
					if (OccupiedGrid[ItemPosition.GridID].IsValidIndex(Index))
					{
						OccupiedGrid[ItemPosition.GridID][Index] = IsOccupy;
					}
				}
			}
		}
	}
	// 打印当前网格占据图
	// PrintDebugOccupiedGrid();
}

void UGridInvSys_InventoryModule_Container::UpdateContainerGridOccupyState(FIntPoint ItemSize,
	const FGridInvSys_ItemPosition& ItemPosition, bool IsOccupy)
{
	if (ItemPosition.IsValid() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemPosition is not valid."), __FUNCTION__)
		return;
	}
	if (!OccupiedGrid.IsValidIndex(ItemPosition.GridID) || !ContainerGridSize.IsValidIndex(ItemPosition.GridID))
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, GridID[%d] is not valid index."), __FUNCTION__, ItemPosition.GridID)
		return;
	}

	FIntPoint GridSize = ContainerGridSize[ItemPosition.GridID];
	int32 ItemMax_X = ItemPosition.Position.X + ItemSize.X;
	int32 ItemMax_Y = ItemPosition.Position.Y + ItemSize.Y;
	if (GridSize.X < ItemMax_X || GridSize.Y < ItemMax_Y)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品大小超出范围."), __FUNCTION__)
		return;
	}

	for (int X = ItemPosition.Position.X; X < ItemMax_X; ++X)
	{
		for (int Y = ItemPosition.Position.Y; Y < ItemMax_Y; ++Y)
		{
			int32 Index = X * GridSize.Y + Y;
			if (OccupiedGrid[ItemPosition.GridID].IsValidIndex(Index))
			{
				OccupiedGrid[ItemPosition.GridID][Index] = IsOccupy;
			}
			else
			{
				UE_LOG(LogInventorySystem, Warning, TEXT("[%d:%d] is not valid index."), ItemPosition.GridID, Index);
			}
		}
	}
}

bool UGridInvSys_InventoryModule_Container::FindEmptyPosition(FIntPoint ItemSize,
                                                                FGridInvSys_ItemPosition& OutPosition)
{
	// 循环容器内所有的网格
	for (int GridID = 0; GridID < OccupiedGrid.Num(); ++GridID)
	{
		const TArray<bool>& GridItems = OccupiedGrid[GridID];
		for (int i = 0; i < GridItems.Num(); ++i) 
		{
			if (GridItems[i] == true) continue; // 遍历网格内所有位置，检查是否未被占领

			FIntPoint ToPosition = FIntPoint(i / ContainerGridSize[GridID].Y, i % ContainerGridSize[GridID].Y);
			if (IsUnoccupiedInSquareRange(GridID, ToPosition, ItemSize))
			{
				OutPosition.EquipSlotTag = GetInventoryTag();
				OutPosition.GridID = GridID;
				OutPosition.Position = ToPosition;
				// OutPosition.Direction = EGridInvSys_ItemDirection::Horizontal; // 外部决定方向，因为仅通过Size无法确认它的方向
				return true;
			}
		}
	}

	return false;
}

void UGridInvSys_InventoryModule_Container::OnContainerPostAdd(UInvSys_InventoryItemInstance* ItemInstance)
{
	Super::OnContainerPostAdd(ItemInstance);
	if (ItemInstance)
	{
		auto GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		check(GridItemInstance)
		UpdateContainerGridItemState(GridItemInstance, GridItemInstance->GetItemPosition(), true);
	}
}

void UGridInvSys_InventoryModule_Container::OnContainerPreRemove(UInvSys_InventoryItemInstance* ItemInstance)
{
	Super::OnContainerPreRemove(ItemInstance);
	if (ItemInstance)
	{
		auto GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		check(GridItemInstance)
		UpdateContainerGridItemState(GridItemInstance, GridItemInstance->GetItemPosition(), false);
	}
}

void UGridInvSys_InventoryModule_Container::UpdateContainerData_FromEquip()
{
	// 当装备物品时，根据物品定义中的容器布局创建临时控件，初始化容器内部数据
	auto WarpEquipItemFunc = [this](FGameplayTag Tag, const FInvSys_EquipItemInstanceMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryTag() && Message.InvComp == GetInventoryComponent())
		{
			auto ContainerLayoutFragment = Message.ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
			if (ContainerLayoutFragment)
			{
				// InventoryWidget::ExposeOnSpawn_InventoryObject = GetInventoryObject();
				UGridInvSys_ContainerGridLayoutWidget* TempContainerLayout =
					CreateWidget<UGridInvSys_ContainerGridLayoutWidget>(GetWorld(), ContainerLayoutFragment->ContainerLayout);
				check(TempContainerLayout)
				if (TempContainerLayout)
				{
					UpdateContainerData(TempContainerLayout);
					TempContainerLayout->ConditionalBeginDestroy(); // 使用完成后标记销毁该对象
				}
			}
		}
	};
	auto WarpUnEquipItemFunc = [this](FGameplayTag Tag, const FInvSys_EquipItemInstanceMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryTag() && Message.InvComp == GetInventoryComponent())
		{
			OccupiedGrid.Empty();
			ContainerGridSize.Empty();
		}
	};
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	OnEquipItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_EquipItemInstanceMessage>(
		Inventory_Message_EquipItem, MoveTemp(WarpEquipItemFunc));
	OnUnEquipItemInstanceHandle = MessageSubsystem.RegisterListener<FInvSys_EquipItemInstanceMessage>(
		Inventory_Message_UnEquipItem, MoveTemp(WarpUnEquipItemFunc));
}

void UGridInvSys_InventoryModule_Container::UpdateContainerData_FromWidget()
{
	if (InventoryObject)
	{
		auto DisplayFragment = InventoryObject->FindInventoryFragment<UInvSys_InventoryModule_Display>();
		if (DisplayFragment)
		{
			// CreateDisplayWidget 函数执行时机是由玩家控制的，故这里直接获取显示控件可能会为空。
			TSubclassOf<UUserWidget> GridLayoutWidgetClass = DisplayFragment->GetDisplayWidgetClass();
			UGridInvSys_ContainerGridLayoutWidget* TempContainerLayout =
				CreateWidget<UGridInvSys_ContainerGridLayoutWidget>(GetWorld(), GridLayoutWidgetClass);
			check(TempContainerLayout)
			if (TempContainerLayout)
			{
				UpdateContainerData(TempContainerLayout);
				TempContainerLayout->ConditionalBeginDestroy(); // 使用完成后及时销毁该对象
			}
		}
	}
}

void UGridInvSys_InventoryModule_Container::UpdateContainerData_FromCustom()
{
	OccupiedGrid.Empty();
	OccupiedGrid.Reserve(CustomContainerLayoutData.Num());
	ContainerGridSize.Empty();
	ContainerGridSize.Reserve(CustomContainerLayoutData.Num());

	ContainerGridSize = CustomContainerLayoutData;
	for (FIntPoint GridSize : CustomContainerLayoutData)
	{
		TArray<bool> TempArry;
		TempArry.Init(false, GridSize.X * GridSize.Y);
		OccupiedGrid.Emplace(TempArry);
	}
}

void UGridInvSys_InventoryModule_Container::UpdateContainerData(UGridInvSys_ContainerGridLayoutWidget* ContainerLayout)
{
	if(ContainerLayout)
	{
		TArray<UGridInvSys_ContainerGridWidget*> AllContainerGrid;
		ContainerLayout->GetAllContainerGridWidgets(AllContainerGrid);

		OccupiedGrid.Empty();
		OccupiedGrid.Reserve(AllContainerGrid.Num());
		ContainerGridSize.Empty();
		ContainerGridSize.Reserve(AllContainerGrid.Num());
		for (const UGridInvSys_ContainerGridWidget* ContainerGrid : AllContainerGrid)
		{
			FIntPoint Size = ContainerGrid->GetContainerGridSize();
			ContainerGridSize.Emplace(Size);
		
			TArray<bool> TempArry;
			TempArry.Init(false, Size.X * Size.Y);
			OccupiedGrid.Emplace(TempArry);
		}
	}
}

void UGridInvSys_InventoryModule_Container::PrintDebugOccupiedGrid(const FString& PrintReason) const
{
#if WITH_EDITOR
	for (int i = 0; i < OccupiedGrid.Num(); ++i)
	{
		int32 Width = ContainerGridSize[i].Y;
		bool bIsFirst = true;
#if WITH_EDITOR
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("[%s:%s]::%s_%d === { Size = %s }"), *GPlayInEditorContextString,
			*GetOwner()->GetName(), *GetInventoryTag().ToString(), i, *ContainerGridSize[i].ToString());
#endif
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("PRINT_REASON::%s"), *PrintReason);
		FString PrintStr = "";
		for (int j = 0; j < OccupiedGrid[i].Num(); ++j)
		{
			bool bIsNewLine = j % Width == 0; //是否为新一行
			if (bIsNewLine && bIsFirst == false)
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("%s"), *PrintStr);
				PrintStr = "";
				PrintStr.Append(OccupiedGrid[i][j] ? TEXT("\tT") : TEXT("\tF"));
			}
			else
			{
				bIsFirst = false;
				PrintStr.Append(OccupiedGrid[i][j] ? TEXT("\tT") : TEXT("\tF"));
			}
		}
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("%s"), *PrintStr);
		PrintStr = "";
	}
#endif
}
