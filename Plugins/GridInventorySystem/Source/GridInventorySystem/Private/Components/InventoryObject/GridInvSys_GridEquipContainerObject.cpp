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
	if (EquipmentSlotWidget)
	{
		// 刷新装备槽
		UE_LOG(LogInventorySystem, Log, TEXT("[%s] 接收到新的装备 [%s]"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *Occupant.ItemID.ToString())

		// todo:: Use interface?
		if (UGridInvSys_EquipmentSlotWidget* GridInvSys_EquipmentSlotWidget = Cast<UGridInvSys_EquipmentSlotWidget>(EquipmentSlotWidget))
		{
			GridInvSys_EquipmentSlotWidget->UpdateOccupant(Occupant);
		}
		TryRefreshContainerItems("TryRefreshOccupant() ===> TryRefreshContainerItems()");
	}
}

void UGridInvSys_GridEquipContainerObject::TryRefreshContainerItems(const FString& Reason)
{
	Super::TryRefreshContainerItems(Reason);
	UE_LOG(LogInventorySystem, Log, TEXT("刷新容器所有内容，ContainerItems 的长度为：%s"), *FString::FromInt(ContainerItems.Num()))
	if (EquipmentSlotWidget)
	{
		// 刷新网格容器
		if (UGridInvSys_EquipContainerSlotWidget* EquipContainerSlotWidget =
			Cast<UGridInvSys_EquipContainerSlotWidget>(EquipmentSlotWidget))
		{
			// 本地缓存容器网格映射，方便通过 GridID 直接获取 ContainerGrid 对象
			TArray<UGridInvSys_ContainerGridWidget*> ContainerGrids;
			EquipContainerSlotWidget->GetAllContainerGrid(ContainerGrids);
			ContainerGridMap.Empty();
			ContainerGridMap.Reserve(ContainerGrids.Num());
			for (UGridInvSys_ContainerGridWidget* ContainerGrid : ContainerGrids)
			{
				ContainerGridMap.Add(ContainerGrid->GetContainerGridID(), ContainerGrid);
			}
		}
		
		if (UGridInvSys_EquipContainerSlotWidget* EquipContainerSlotWidget =
			Cast<UGridInvSys_EquipContainerSlotWidget>(EquipmentSlotWidget))
		{
			EquipContainerSlotWidget->UpdateContainerGrid(ContainerItems);
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
		ContainerItems.Add(InventoryItem);
		ContainerItemMap.Add(InventoryItem.BaseItemData.UniqueID, InventoryItem);
		AddDataToRep_AddedInventoryItems(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::RemoveInventoryItemFromContainer(FGridInvSys_InventoryItem InventoryItem)
{
	// todo::
	if (HasAuthority())
	{
		ContainerItems.Remove(InventoryItem);
		ContainerItemMap.Remove(InventoryItem.BaseItemData.UniqueID);
		AddDataToRep_RemovedInventoryItems(InventoryItem.BaseItemData.UniqueID);
	}
}

void UGridInvSys_GridEquipContainerObject::UpdateInventoryItemFromContainer(FGridInvSys_InventoryItem NewItem,
	FGridInvSys_InventoryItem OldItem)
{
	// todo::
	if (HasAuthority())
	{
		if (ContainerItemMap.Contains(OldItem.BaseItemData.UniqueID))
		{
			// todo:: 优化为 Update
			ContainerItems.Remove(OldItem);
			ContainerItems.Add(NewItem);
			ContainerItemMap[OldItem.BaseItemData.UniqueID] = NewItem;
			AddDataToRep_ChangedInventoryItems(OldItem.BaseItemData.UniqueID);
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

	DOREPLIFETIME(UGridInvSys_GridEquipContainerObject, ContainerItems);
}

void UGridInvSys_GridEquipContainerObject::OnAddedContainerItems(const TArray<FName>& InAddedItems)
{
	if (EquipmentSlotWidget)
	{
		for (FName ItemUniqueID : InAddedItems)
		{
			// ContainerItems 是服务器同步过来的，所以在正常情况下，其内部应该会存在
			int32 Index = FindContainerItemIndex(ItemUniqueID);
			if (ContainerItems.IsValidIndex(Index))
			{
				UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridMap[ContainerItems[Index].ItemPosition.GridID];
				GridWidget->AddInventoryItemTo(ContainerItems[Index]);
			}
		}
	}
}

void UGridInvSys_GridEquipContainerObject::OnRemovedContainerItems(const TArray<FName>& InRemovedItems)
{
	if (EquipmentSlotWidget)
	{
		for (FName ItemUniqueID : InRemovedItems)
		{
			// ContainerItems 是服务器同步过来的，所以在正常情况下，其内部应该会存在
			int32 Index = FindContainerItemIndex(ItemUniqueID);
			if (ContainerItems.IsValidIndex(Index))
			{
				UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridMap[ContainerItems[Index].ItemPosition.GridID];
				// GridWidget->AddInventoryItemTo(ContainerItems[Index]);
				// RemoveInventoryItem
			}
		}
	}
}

void UGridInvSys_GridEquipContainerObject::OnUpdatedContainerItems(const TArray<FName>& InChangedItems)
{
	Super::OnUpdatedContainerItems(InChangedItems);
}

int32 UGridInvSys_GridEquipContainerObject::FindContainerItemIndex(FName ItemUniqueID)
{
	// todo:: 优化查询速度
	for (int i = 0; i < ContainerItems.Num(); ++i)
	{
		if (ContainerItems[i].BaseItemData.UniqueID == ItemUniqueID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
