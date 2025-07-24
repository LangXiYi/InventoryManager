// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/GridInvSys_GridEquipContainerObject.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Data/GridInvSys_InventoryContainerInfo.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "Widgets/GridInvSys_EquipContainerSlotWidget.h"


UGridInvSys_GridEquipContainerObject::UGridInvSys_GridEquipContainerObject()
{
}

void UGridInvSys_GridEquipContainerObject::AddInventoryItemToEquipSlot_DEPRECATED(const FInvSys_InventoryItem& NewItem)
{
	if (HasAuthority() == false)
	{
		return;
	}
	// 根据Item信息获取容器网格信息，比如各个网格的大小等数据。
	if (UGridInvSys_InventoryContainerInfo* ContainerInfo = Cast<UGridInvSys_InventoryContainerInfo>(NewItem.ItemInfo))
	{
		ContainerGridSizeMap = ContainerInfo->ContainerGridSizeMap;
	}
	// 检查类型是否一致
	if (EquipmentSupportType == EGridInvSys_InventoryItemType::Weapon_Primary)
	{
		// 根据 NewItem.ItemID 判断物品类型
		// GetItemType(ItemID)
	}
	Super::AddInventoryItemToEquipSlot_DEPRECATED(NewItem);
}

void UGridInvSys_GridEquipContainerObject::AddInventoryItemToContainer_DEPRECATED(const FGridInvSys_InventoryItem& InventoryItem)
{
	if (HasAuthority())
	{
		RepNotify_ContainerItems_DEPRECATED.Add(InventoryItem);
		// ContainerGridItems.Add(InventoryItem.BaseItemData.UniqueID, InventoryItem);
		ItemPositionMap.Add(InventoryItem.ItemPosition.Position, InventoryItem.BaseItemData.UniqueID);
		//RecordItemOperationByAdd(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::RemoveInventoryItemFromContainer_DEPRECATED(FGridInvSys_InventoryItem InventoryItem)
{
	// todo::
	if (HasAuthority())
	{
		RepNotify_ContainerItems_DEPRECATED.Remove(InventoryItem);
		// ContainerGridItems.Remove(InventoryItem.BaseItemData.UniqueID);
		ItemPositionMap.Remove(InventoryItem.ItemPosition.Position);
		//RecordItemOperationByRemove(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::UpdateInventoryItemFromContainer_DEPRECATED(FName ItemUniqueID, FGridInvSys_InventoryItemPosition NewPosition)
{
	if (HasAuthority())
	{
		if (ContainerGridItems.Contains(ItemUniqueID))
		{
			FGridInvSys_InventoryItem TempGridItem = ContainerGridItems[ItemUniqueID];
			ItemPositionMap.Remove(TempGridItem.ItemPosition.Position);
			ItemPositionMap.Add(NewPosition.Position, ItemUniqueID);

			const int32 Index = RepNotify_ContainerItems_DEPRECATED.Find(TempGridItem);
			TempGridItem.ItemPosition = NewPosition;
			TempGridItem.BaseItemData.SlotName = NewPosition.SlotName;
			RepNotify_ContainerItems_DEPRECATED[Index] = TempGridItem;
			// ContainerGridItems[ItemUniqueID] = TempGridItem;
			//RecordItemOperationByUpdate(ItemUniqueID);
		}
	}
}

UInvSys_EquipSlotWidget* UGridInvSys_GridEquipContainerObject::CreateDisplayWidget(APlayerController* PC)
{
	UInvSys_EquipSlotWidget* LOCAL_EquipSlotWidget = Super::CreateDisplayWidget(PC);
	if (LOCAL_EquipSlotWidget && LOCAL_EquipSlotWidget->IsA<UGridInvSys_EquipContainerSlotWidget>())
	{
		UGridInvSys_EquipContainerSlotWidget* LOCAL_GridEquipSlowWidget = Cast<UGridInvSys_EquipContainerSlotWidget>(LOCAL_EquipSlotWidget);
		// LOCAL_GridEquipSlowWidget->SetEquipItemType(EquipmentSupportType);
	}
	return LOCAL_EquipSlotWidget;
}

void UGridInvSys_GridEquipContainerObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipContainerObject, EquipmentSupportType);
}

void UGridInvSys_GridEquipContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UGridInvSys_GridEquipContainerObject::OnItemPositionChange(const FGridInvSys_ItemPositionChangeMessage& Message)
{
#if 0
	if (ContainerLayout == nullptr) return;

	UGridInvSys_ContainerGridLayoutWidget* LOCAL_ContainerLayout = nullptr;
	if (ContainerLayout->IsA(UGridInvSys_ContainerGridLayoutWidget::StaticClass()))
	{
		LOCAL_ContainerLayout = Cast<UGridInvSys_ContainerGridLayoutWidget>(ContainerLayout);
	}
	if (LOCAL_ContainerLayout == nullptr) return; //目标控件类型不匹配。

	// UE_LOG(LogInventorySystem, Warning, TEXT("==== OnItemPositionChange [%s:Begin] ===="), HasAuthority() ? TEXT("Server"):TEXT("Client"))
	if (UGridInvSys_ContainerGridItemWidget* ItemWidget = LOCAL_ContainerLayout->FindGridItemWidget(Message.OldPosition)) 
	{
		// 目标位置存在物品且待移除的物品也要一致。
		if (ItemWidget->IsOccupied() == true && ItemWidget->GetItemInstance() == Message.Instance)
		{
			UE_LOG(LogInventorySystem, Log, TEXT("正在移除旧位置的物品==>[%s]"), *Message.OldPosition.ToString())
			ItemWidget->RemoveItemInstance();
		}
	}
	
	if (UGridInvSys_ContainerGridItemWidget* ItemWidget = LOCAL_ContainerLayout->FindGridItemWidget(Message.NewPosition))
	{
		if (ItemWidget->IsOccupied() == false)
		{
			UE_LOG(LogInventorySystem, Log, TEXT("正在为新位置添加物品==>[%s]"), *Message.NewPosition.ToString())
			ItemWidget->AddItemInstance(Message.Instance);
		}
		else if(ItemWidget->GetItemInstance() != Message.Instance)
		{
			check(Message.Instance)
			if (Message.Instance && Message.Instance->IsA<UGridInvSys_InventoryItemInstance>())
			{
				// 添加操作失败了，所以需要解除之前设置的代理绑定，然后将它还原到原来的位置。
				UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(Message.Instance);
				GridItemInstance->OnItemPositionChangeDelegate().Unbind();
			}
			check(false)
			UGridInvSys_InventoryComponent* PlayerInvComp = GetWorld()->GetFirstPlayerController()->GetComponentByClass<UGridInvSys_InventoryComponent>();
			PlayerInvComp->Server_RestoreItemInstanceToPos(Message.Instance->GetLastInventoryComponent(), Message.Instance, Message.OldPosition);
			UE_LOG(LogInventorySystem, Error, TEXT("[%s]位置已经被其他物品占用！！正在回退至%s")
				, HasAuthority() ? TEXT("Server"):TEXT("Client"), *Message.OldPosition.ToString())
		}
	}
#endif
}

void UGridInvSys_GridEquipContainerObject::OnInventoryStackChange(const FInvSys_InventoryStackChangeMessage& ChangeInfo)
{
	if (ContainerLayout == nullptr) return;

	UGridInvSys_ContainerGridLayoutWidget* LOCAL_ContainerLayout = nullptr;
	if (ContainerLayout.IsA(UGridInvSys_ContainerGridLayoutWidget::StaticClass()))
	{
		LOCAL_ContainerLayout = Cast<UGridInvSys_ContainerGridLayoutWidget>(ContainerLayout);
	}
	if (LOCAL_ContainerLayout == nullptr) return; //目标控件类型不匹配。

	if (UGridInvSys_ContainerGridItemWidget* ItemWidget = LOCAL_ContainerLayout->FindGridItemWidget(ChangeInfo.ItemInstance)) 
	{
		//todo::更新数量显示
		// UE_LOG(LogInventorySystem, Warning, TEXT("正在更新物品数量 ==> [%d]"), ChangeInfo.StackCount);
		// ItemWidget->RemoveItemInstance();
	}
}

void UGridInvSys_GridEquipContainerObject::OnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsForceRep)
{
	if (Entry.Instance)
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(Entry.Instance);
		UpdateContainerGridItemState(GridItemInstance, true);
	}
}

void UGridInvSys_GridEquipContainerObject::OnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsForceRep)
{
	if (Entry.Instance)
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(Entry.Instance);
		UpdateContainerGridItemState(GridItemInstance, false);
	}
}

void UGridInvSys_GridEquipContainerObject::NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	Super::NativeOnEquipItemInstance(InItemInstance);

	if(ContainerLayout)
	{
		OccupiedGrid.Empty();
		ContainerGridSize.Empty();
		UGridInvSys_ContainerGridLayoutWidget* ContainerGridLayout = Cast<UGridInvSys_ContainerGridLayoutWidget>(ContainerLayout);
		if (ContainerGridLayout)
		{
			TArray<UGridInvSys_ContainerGridWidget*> OutContainerGrids;
			ContainerGridLayout->GetAllContainerGridWidgets(OutContainerGrids);
			UE_LOG(LogInventorySystem, Log, TEXT("正在初始化容器各个网格大小[%d]"), OutContainerGrids.Num())
			for (const UGridInvSys_ContainerGridWidget* ContainerGrid : OutContainerGrids)
			{
				FIntPoint Size = ContainerGrid->GetContainerGridSize();
				ContainerGridSize.Add(Size);
			
				TArray<bool> TempArry;
				TempArry.Init(false, Size.X * Size.Y);
				OccupiedGrid.Add(TempArry);
			}
		}
	}

}

void UGridInvSys_GridEquipContainerObject::NativeOnUnEquipItemInstance()
{
	Super::NativeOnUnEquipItemInstance();
	OccupiedGrid.Empty();
	ContainerGridSize.Empty();
}

void UGridInvSys_GridEquipContainerObject::UpdateContainerGridItemState(
	UGridInvSys_InventoryItemInstance* GridItemInstance, bool IsOccupy)
{
	if (GridItemInstance)
	{
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
	}
}

int32 UGridInvSys_GridEquipContainerObject::FindContainerItemIndex(FName ItemUniqueID)
{
	// todo:: 优化查询速度
	for (int i = 0; i < RepNotify_ContainerItems_DEPRECATED.Num(); ++i)
	{
		if (RepNotify_ContainerItems_DEPRECATED[i].BaseItemData.UniqueID == ItemUniqueID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UGridInvSys_GridEquipContainerObject::HasEnoughFreeSpace(FIntPoint ToPos, int32 ToGridID, FIntPoint ItemSize)
{
	int32 ToIndex = ToPos.X * ContainerGridSize[ToGridID].Y + ToPos.Y;
	int32 MaxGridWidth = ContainerGridSize[ToGridID].Y;
	int32 MaxGridHeight = ContainerGridSize[ToGridID].Y;
	for (int X = 0; X < ItemSize.X; ++X)
	{
		for (int Y = 0; Y < ItemSize.Y; ++Y)
		{
			if (ToPos.X + X >= MaxGridWidth || ToPos.Y + Y >= MaxGridHeight)
			{
				return false;
			}
			int32 Index = ToIndex + X * ContainerGridSize[ToGridID].Y + Y;
			if (OccupiedGrid[ToGridID].IsValidIndex(Index) == false ||
				OccupiedGrid[ToGridID][Index] == true)
			{
				return false;
			}
		}
	}
	return true;
}

bool UGridInvSys_GridEquipContainerObject::FindEmptyPosition(FIntPoint ItemSize,
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
				OutPosition.EquipSlotTag = EquipSlotTag;
				OutPosition.GridID = GridID;
				OutPosition.Position = ToPosition;
				// OutPosition.Direction = EGridInvSys_ItemDirection::Horizontal; // 外部决定方向，因为仅通过Size无法确认它的方向
				return true;
			}
		}
	}

	return false;
}

TArray<bool> UGridInvSys_GridEquipContainerObject::GetInventoryOccupyStateByGridID(int32 GridID)
{
	if (OccupiedGrid.IsValidIndex(GridID) == false)
	{
		checkNoEntry();
		return TArray<bool>();
	}
	return OccupiedGrid[GridID];
}

bool UGridInvSys_GridEquipContainerObject::IsValidPosition(const FGridInvSys_InventoryItemPosition& ItemPosition)
{
	if (HasAuthority() == false)
	{
		return false;
	}
	
	if (ContainerGridSizeMap.Contains(ItemPosition.GridID) == false)
	{
		return false;
	}
	// 根据GridID检查对应网格的范围，判断该物品是否可以存放在当前位置
	int32 ItemMaxPosX = ItemPosition.Position.X + ItemPosition.ItemSize.X - 1;
	int32 ItemMaxPosY = ItemPosition.Position.Y + ItemPosition.ItemSize.Y - 1;
	FIntPoint GridSize = ContainerGridSizeMap[ItemPosition.GridID];
	if (ItemPosition.Position.X < 0 || ItemMaxPosX > GridSize.X ||
		ItemPosition.Position.Y < 0 || ItemMaxPosX > GridSize.Y)
	{
		return false;
	}
	// 判断目标范围内是否存在其他物品 
	for (int i = ItemPosition.Position.X; i < ItemMaxPosX; ++i)
	{
		for (int j = ItemPosition.Position.Y; j < ItemMaxPosY; ++j)
		{
			if (ItemPositionMap.Contains(FIntPoint(i, j)))
			{
				return false;
			}
		}
	}
	return true;
}

bool UGridInvSys_GridEquipContainerObject::FindEnoughFreeSpace(FIntPoint ItemSize,
                                                               FGridInvSys_InventoryItemPosition& OutPosition) const
{
	if (EquipSlotWidget && EquipSlotWidget.IsA(UGridInvSys_EquipContainerSlotWidget::StaticClass()))
	{
		for (auto ContainerGridWidget : ContainerGridWidgets)
		{
			FIntPoint TempPosition;
			if (ContainerGridWidget.Value->FindValidPosition(ItemSize, TempPosition))
			{
				// 根据找到的有效位置设置值。
				OutPosition.GridID = ContainerGridWidget.Key;
				OutPosition.Position = TempPosition;
				return true;
			}
		}
	}
	return false;
}

bool UGridInvSys_GridEquipContainerObject::FindContainerGridItem(const FIntPoint& ItemPosition, FGridInvSys_InventoryItem& OutItem) const
{
	if (ItemPositionMap.Contains(ItemPosition))
	{
		const FName ItemUniqueID = ItemPositionMap[ItemPosition];
		if (ContainerGridItems.Contains(ItemUniqueID))
		{
			OutItem = ContainerGridItems[ItemUniqueID];
			return true;
		}
	}
	return false;
}

bool UGridInvSys_GridEquipContainerObject::FindContainerGridItem(FName ItemUniqueID, FGridInvSys_InventoryItem& OutItem) const
{
	if (ContainerGridItems.Contains(ItemUniqueID))
	{
		OutItem = ContainerGridItems[ItemUniqueID];
		return true;
	}
	return false;
}
