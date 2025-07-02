// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/GridInvSys_GridEquipContainerObject.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"
#include "Widgets/GridInvSys_EquipContainerSlotWidget.h"



UGridInvSys_GridEquipContainerObject::UGridInvSys_GridEquipContainerObject()
{
}

void UGridInvSys_GridEquipContainerObject::TryRefreshOccupant(const FString& Reason)
{
	Super::TryRefreshOccupant(Reason);
	if (EquipmentSlotWidget && IsEquipped())
	{
		// 刷新装备槽
		UE_LOG(LogInventorySystem, Log, TEXT("[%s] 刷新装备槽 [%s]"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *SlotName.ToString())

		// todo:: Use interface?
		if (UGridInvSys_EquipmentSlotWidget* GridInvSys_EquipmentSlotWidget = Cast<UGridInvSys_EquipmentSlotWidget>(EquipmentSlotWidget))
		{
			GridInvSys_EquipmentSlotWidget->UpdateOccupant(Occupant);
		}
		TryRefreshContainerItems(Reason);
	}
}

void UGridInvSys_GridEquipContainerObject::TryRefreshContainerItems(const FString& Reason)
{
	Super::TryRefreshContainerItems(Reason);
	UE_LOG(LogInventorySystem, Log, TEXT("刷新容器所有内容，ContainerItems 的长度为：%s"), *FString::FromInt(RepNotify_ContainerItems.Num()))
	if (EquipmentSlotWidget)
	{
		// 刷新网格容器
		if (UGridInvSys_EquipContainerSlotWidget* EquipContainerSlotWidget =
			Cast<UGridInvSys_EquipContainerSlotWidget>(EquipmentSlotWidget))
		{
			// 本地缓存容器网格映射，方便通过 GridID 直接获取 ContainerGrid 对象
			TArray<UGridInvSys_ContainerGridWidget*> ContainerGrids;
			EquipContainerSlotWidget->GetAllContainerGrid(ContainerGrids);
			ContainerGridWidgets.Empty();
			ContainerGridWidgets.Reserve(ContainerGrids.Num());
			for (UGridInvSys_ContainerGridWidget* ContainerGrid : ContainerGrids)
			{
				ContainerGridWidgets.Add(ContainerGrid->GetContainerGridID(), ContainerGrid);
			}
			EquipContainerSlotWidget->UpdateContainerGrid(RepNotify_ContainerItems);
		}
	}
}

void UGridInvSys_GridEquipContainerObject::CreateDisplayWidget(APlayerController* PC)
{
	Super::CreateDisplayWidget(PC);
#if WITH_EDITOR
	if (InventoryComponent->IsA(UGridInvSys_InventoryComponent::StaticClass()) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("库存组件类型与填充的 InventoryObject 的类型不匹配。"))
		return;
	}
#endif
	UGridInvSys_InventoryComponent* GridInventoryComponent = static_cast<UGridInvSys_InventoryComponent*>(InventoryComponent);

	if (PC)
		EquipmentSlotWidget = CreateWidget<UGridInvSys_EquipmentSlotWidget>(PC, EquipmentSlotWidgetClass);
	else
		EquipmentSlotWidget = CreateWidget<UGridInvSys_EquipmentSlotWidget>(GetWorld(), EquipmentSlotWidgetClass);

	EquipmentSlotWidget->SetSlotName(GetSlotName());
	EquipmentSlotWidget->SetInventoryComponent(GetInventoryComponent());

	// 获取库存布局控件，并将装备槽插入指定位置
	UUserWidget* InventoryMenuWidget = GridInventoryComponent->GetInventoryLayoutWidget();
	if (InventoryMenuWidget == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("InventoryMenuWidget 在 GridInventoryComponent 中不存在"));
		return;
	}
	UNamedSlot* NamedSlot = Cast<UNamedSlot>(InventoryMenuWidget->GetWidgetFromName(GetSlotName()));
#if WITH_EDITOR
	if (NamedSlot == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("命名槽[%s]在 InventoryMenuWidget 中不存在"), *GetSlotName().ToString())
		return;
	}
#endif

	NamedSlot->AddChild(EquipmentSlotWidget);
	// TryRefreshOccupant();
}

void UGridInvSys_GridEquipContainerObject::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem)
{
	// 检查类型是否一致
	if (EquipmentSupportType == EGridInvSys_InventoryItemType::Weapon_Primary)
	{
		// 根据 NewItem.ItemID 判断物品类型
		// GetItemType(ItemID)
	}
	Super::AddInventoryItemToEquipSlot(NewItem);
}

void UGridInvSys_GridEquipContainerObject::AddInventoryItemToContainer(const FGridInvSys_InventoryItem& InventoryItem)
{
	if (HasAuthority())
	{
		RepNotify_ContainerItems.Add(InventoryItem);
		ContainerGridItems.Add(InventoryItem.BaseItemData.UniqueID, InventoryItem);
		ItemPositionMap.Add(InventoryItem.ItemPosition.Position, InventoryItem.BaseItemData.UniqueID);
		AddDataToRep_AddedInventoryItems(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::RemoveInventoryItemFromContainer(FGridInvSys_InventoryItem InventoryItem)
{
	// todo::
	if (HasAuthority())
	{
		RepNotify_ContainerItems.Remove(InventoryItem);
		ContainerGridItems.Remove(InventoryItem.BaseItemData.UniqueID);
		ItemPositionMap.Remove(InventoryItem.ItemPosition.Position);
		AddDataToRep_RemovedInventoryItems(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::UpdateInventoryItemFromContainer(FName ItemUniqueID, FGridInvSys_InventoryItemPosition NewPosition)
{
	if (HasAuthority())
	{
		if (ContainerGridItems.Contains(ItemUniqueID))
		{
			FGridInvSys_InventoryItem TempGridItem = ContainerGridItems[ItemUniqueID];
			ItemPositionMap.Remove(TempGridItem.ItemPosition.Position);
			ItemPositionMap.Add(NewPosition.Position, ItemUniqueID);

			const int32 Index = RepNotify_ContainerItems.Find(TempGridItem);
			TempGridItem.ItemPosition = NewPosition;
			TempGridItem.BaseItemData.SlotName = NewPosition.SlotName;
			RepNotify_ContainerItems[Index] = TempGridItem;
			ContainerGridItems[ItemUniqueID] = TempGridItem;
			AddDataToRep_ChangedInventoryItems(ItemUniqueID);
		}
	}
}

void UGridInvSys_GridEquipContainerObject::CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,
	UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(NewInventoryComponent, PreEditPayLoad);

	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipContainerObject, EquipmentSlotWidgetClass);
	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipContainerObject, EquipmentSupportType);
}

void UGridInvSys_GridEquipContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_GridEquipContainerObject, RepNotify_ContainerItems);
}

void UGridInvSys_GridEquipContainerObject::OnAddedContainerItems(const TArray<FName>& InAddedItems)
{
	for (FName ItemUniqueID : InAddedItems)
	{
		// ContainerItems 是服务器同步过来的，所以在正常情况下，其内部应该会存在
		int32 Index = FindContainerItemIndex(ItemUniqueID);
		if (RepNotify_ContainerItems.IsValidIndex(Index))
		{
			// 确保未显示控件时，物品信息任然存在
			FGridInvSys_InventoryItem TempInvItem = RepNotify_ContainerItems[Index];
			ContainerGridItems.Add(ItemUniqueID, TempInvItem);

			if (EquipmentSlotWidget)
			{
				UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridWidgets[TempInvItem.ItemPosition.GridID];
				GridWidget->UpdateInventoryItem(RepNotify_ContainerItems[Index]);
			}
		}
	}
}

void UGridInvSys_GridEquipContainerObject::OnRemovedContainerItems(const TArray<FName>& InRemovedItems)
{

	for (FName ItemUniqueID : InRemovedItems)
	{
		if (ContainerGridItems.Contains(ItemUniqueID))
		{
			FGridInvSys_InventoryItem TempItemData = ContainerGridItems[ItemUniqueID];
			ContainerGridItems.Remove(ItemUniqueID);

			if (EquipmentSlotWidget == nullptr)
			{
				UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridWidgets[TempItemData.ItemPosition.GridID];
				GridWidget->RemoveInventoryItem(TempItemData);
			}
		}
	}
}

void UGridInvSys_GridEquipContainerObject::OnUpdatedContainerItems(const TArray<FName>& InChangedItems)
{

	for (FName ItemUniqueID : InChangedItems)
	{
		int32 Index = FindContainerItemIndex(ItemUniqueID);
		if (!RepNotify_ContainerItems.IsValidIndex(Index))
		{
			check(false);
			continue;
		}
		const FGridInvSys_InventoryItem& NewInvItem = RepNotify_ContainerItems[Index];

		if (EquipmentSlotWidget)
		{
			check(ContainerGridWidgets.Contains(NewInvItem.ItemPosition.GridID))
			if (UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridWidgets[NewInvItem.ItemPosition.GridID])
			{
				if (ContainerGridItems.Contains(ItemUniqueID))
				{
					FGridInvSys_InventoryItem OldInvItem = ContainerGridItems[ItemUniqueID];
					GridWidget->RemoveInventoryItem(OldInvItem); // 移除旧数据
					GridWidget->UpdateInventoryItem(NewInvItem); // 添加新数据
				}
				else
				{
					UE_LOG(LogInventorySystem, Warning, TEXT("%s 在 ContainerGridItems[%d] 中不存在。"), *ItemUniqueID.ToString(), ContainerGridItems.Num())
				}
			}
		}
		// 更新容器保存的物品信息
		ContainerGridItems[ItemUniqueID] = NewInvItem;
	}
	Super::OnUpdatedContainerItems(InChangedItems);
}

int32 UGridInvSys_GridEquipContainerObject::FindContainerItemIndex(FName ItemUniqueID)
{
	// todo:: 优化查询速度
	for (int i = 0; i < RepNotify_ContainerItems.Num(); ++i)
	{
		if (RepNotify_ContainerItems[i].BaseItemData.UniqueID == ItemUniqueID)
		{
			return i;
		}
	}
	return INDEX_NONE;
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
