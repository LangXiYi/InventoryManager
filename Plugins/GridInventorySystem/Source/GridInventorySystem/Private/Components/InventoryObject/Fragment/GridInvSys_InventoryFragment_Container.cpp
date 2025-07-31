// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"

#include "GridInvSys_CommonType.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "BaseInventorySystem.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"

void UGridInvSys_InventoryFragment_Container::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);

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
}

bool UGridInvSys_InventoryFragment_Container::UpdateItemInstancePosition(
	UGridInvSys_InventoryItemInstance* GridItemInstance, const FGridInvSys_ItemPosition& NewPosition)
{
	check(GridItemInstance)
	if (GridItemInstance)
	{
		FIntPoint Size = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(GridItemInstance, NewPosition.Direction);
		if (HasEnoughFreeSpace(NewPosition.Position, NewPosition.GridID, Size))
		{
			GridItemInstance->SetItemPosition(NewPosition);
			MarkItemInstanceDirty(GridItemInstance);
			Owner_Private->ForceNetUpdate();
			return true;
		}
	}
	return false;
}

bool UGridInvSys_InventoryFragment_Container::HasEnoughFreeSpace(
	FIntPoint ToPosition, int32 ToGridID, FIntPoint ItemSize)
{
	int32 ToIndex = ToPosition.X * ContainerGridSize[ToGridID].Y + ToPosition.Y;
	int32 MaxGridWidth = ContainerGridSize[ToGridID].Y;
	int32 MaxGridHeight = ContainerGridSize[ToGridID].Y;
	for (int X = 0; X < ItemSize.X; ++X)
	{
		for (int Y = 0; Y < ItemSize.Y; ++Y)
		{
			if (ToPosition.X + X >= MaxGridWidth || ToPosition.Y + Y >= MaxGridHeight)
			{
				return false;
			}
			int32 Index = ToIndex + X * ContainerGridSize[ToGridID].Y + Y;
			if (OccupiedGrid[ToGridID].IsValidIndex(Index) == false ||
				OccupiedGrid[ToGridID][Index] == true)
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning,
					TEXT("[%s:%d >>> %s] 无法容纳大小为 %s 的物品"), *GetInventoryObjectTag().ToString(), ToGridID,
					*ToPosition.ToString(), *ItemSize.ToString())
				if (PRINT_INVENTORY_SYSTEM_LOG)
				{
					// 打印当前网格占据图
					PrintDebugOccupiedGrid();
				}
				return false;
			}
		}
	}
	return true;
}

void UGridInvSys_InventoryFragment_Container::UpdateContainerGridItemState(
	UGridInvSys_InventoryItemInstance* GridItemInstance, bool IsOccupy)
{
	if (GridItemInstance == nullptr)
	{
		return;
	}
	auto ItemSizeFragment = GridItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
	if (GridItemInstance && ItemSizeFragment)
	{
		FGridInvSys_ItemPosition ItemPosition = GridItemInstance->GetItemPosition();
		FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(GridItemInstance);
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

bool UGridInvSys_InventoryFragment_Container::FindEmptyPosition(FIntPoint ItemSize,
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
			if (HasEnoughFreeSpace(ToPosition, GridID, ItemSize))
			{
				OutPosition.EquipSlotTag = GetInventoryObjectTag();
				OutPosition.GridID = GridID;
				OutPosition.Position = ToPosition;
				// OutPosition.Direction = EGridInvSys_ItemDirection::Horizontal; // 外部决定方向，因为仅通过Size无法确认它的方向
				return true;
			}
		}
	}

	return false;
}

void UGridInvSys_InventoryFragment_Container::RemoveAllItemInstance()
{
	Super::RemoveAllItemInstance();
	// 循环容器内所有的网格, 清空占用状态
	for (int GridID = 0; GridID < OccupiedGrid.Num(); ++GridID)
	{
		TArray<bool>& GridItems = OccupiedGrid[GridID];
		for (int i = 0; i < GridItems.Num(); ++i) 
		{
			GridItems[i] = false;
		}
	}
}

void UGridInvSys_InventoryFragment_Container::NativeOnContainerEntryAdded(
	FInvSys_InventoryItemChangedMessage ChangeInfo)
{
	Super::NativeOnContainerEntryAdded(ChangeInfo);
	if (ChangeInfo.ItemInstance)
	{
		auto GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ChangeInfo.ItemInstance);
		check(GridItemInstance)
		UpdateContainerGridItemState(GridItemInstance, true);
	}
}

void UGridInvSys_InventoryFragment_Container::NativeOnContainerEntryRemove(
	FInvSys_InventoryItemChangedMessage ChangeInfo)
{
	Super::NativeOnContainerEntryRemove(ChangeInfo);
	if (ChangeInfo.ItemInstance)
	{
		auto GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ChangeInfo.ItemInstance);
		check(GridItemInstance)
		UpdateContainerGridItemState(GridItemInstance, false);
	}
}

void UGridInvSys_InventoryFragment_Container::UpdateContainerData_FromEquip()
{
	// 当装备物品时，根据物品定义中的容器布局创建临时控件，初始化容器内部数据
	auto WarpEquipItemFunc = [this](FGameplayTag Tag, const FInvSys_EquipItemInstanceMessage& Message)
	{
		if (Message.InventoryObjectTag == GetInventoryObjectTag() && Message.InvComp == GetInventoryComponent())
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
		if (Message.InventoryObjectTag == GetInventoryObjectTag() && Message.InvComp == GetInventoryComponent())
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

void UGridInvSys_InventoryFragment_Container::UpdateContainerData_FromWidget()
{
	if (InventoryObject)
	{
		auto DisplayFragment = InventoryObject->FindInventoryFragment<UInvSys_InventoryFragment_DisplayWidget>();
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

void UGridInvSys_InventoryFragment_Container::UpdateContainerData_FromCustom()
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

void UGridInvSys_InventoryFragment_Container::UpdateContainerData(UGridInvSys_ContainerGridLayoutWidget* ContainerLayout)
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

void UGridInvSys_InventoryFragment_Container::PrintDebugOccupiedGrid()
{
	for (int i = 0; i < OccupiedGrid.Num(); ++i)
	{
		int32 Width = ContainerGridSize[i].Y;
		bool bIsFirst = true;
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("[%s:%s] Container Grid ID = %d === { Size = %s }"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName(), i, *ContainerGridSize[i].ToString());
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
}
