// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_EquipSlotWidget.h"

#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

void UInvSys_EquipSlotWidget::UpdateEquipItem(UInvSys_InventoryItemInstance* NewItemInstance)
{
	ItemInstance = NewItemInstance;
	if (NewItemInstance != nullptr)
	{
		OnEquipInventoryItem(NewItemInstance);
	}
	else
	{
		OnUnEquipInventoryItem();
	}
}

void UInvSys_EquipSlotWidget::SetInventoryObject(UInvSys_BaseEquipmentObject* NewInvObj)
{
	InvObj = NewInvObj;
	InventoryComponent = NewInvObj->GetInventoryComponent();
	SlotTag = NewInvObj->GetSlotTag();
}
