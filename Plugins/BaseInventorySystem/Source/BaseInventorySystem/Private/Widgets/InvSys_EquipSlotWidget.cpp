// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_EquipSlotWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Interface/InvSys_DraggingItemInterface.h"
#include "Widgets/InvSys_InventoryItemWidget.h"

void UInvSys_EquipSlotWidget::EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (NewItemInstance)
	{
		ItemInstance = NewItemInstance;
		if (NewItemInstance != nullptr)
		{
			OnEquipInventoryItem(NewItemInstance);
		}
	}
}

void UInvSys_EquipSlotWidget::UnEquipItemInstance()
{
	ItemInstance = nullptr;
	OnUnEquipInventoryItem();
}

void UInvSys_EquipSlotWidget::SetInventoryObject(UInvSys_BaseEquipmentObject* NewInvObj)
{
	InvObj = NewInvObj;
	InventoryComponent = NewInvObj->GetInventoryComponent();
	SlotTag = NewInvObj->GetSlotTag();
}

bool UInvSys_EquipSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInvSys_InventoryComponent* PlayerInvComp = GetOwningPlayer()->GetComponentByClass<UInvSys_InventoryComponent>();
	check(PlayerInvComp)
	if (PlayerInvComp == nullptr)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}
	UInvSys_InventoryItemInstance* LOCAL_ItemInstance = IInvSys_DraggingItemInterface::Execute_GetItemInstance(InOperation->DefaultDragVisual);
	check(LOCAL_ItemInstance)
	if (LOCAL_ItemInstance)
	{
		if (CheckIsCanDrop(LOCAL_ItemInstance))
		{
			// 将物品装备至指定槽
			PlayerInvComp->Server_EquipItemInstance(InventoryComponent.Get(), LOCAL_ItemInstance, SlotTag);
		}
		else
		{
			// 还原目标物品
			UInvSys_InventoryComponent* From_InvComp = LOCAL_ItemInstance->GetInventoryComponent();
			PlayerInvComp->Server_RestoreItemInstance(From_InvComp, LOCAL_ItemInstance); 
		}
	}
	return true;
}

bool UInvSys_EquipSlotWidget::CheckIsCanDrop_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	return false;
}
