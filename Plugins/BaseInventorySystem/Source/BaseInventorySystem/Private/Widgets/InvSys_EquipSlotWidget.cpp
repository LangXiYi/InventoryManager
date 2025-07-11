// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_EquipSlotWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryItemInstance.h"
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

	if (ItemInstance == nullptr && InOperation->Payload && InOperation->Payload->IsA<UInvSys_InventoryItemWidget>())
	{
		UInvSys_InventoryItemWidget* ItemWidget = Cast<UInvSys_InventoryItemWidget>(InOperation->Payload);
		UInvSys_InventoryItemInstance* LOCAL_ItemInstance = ItemWidget->GetItemInstance<UInvSys_InventoryItemInstance>();
		if (LOCAL_ItemInstance && CheckIsCanDrop(LOCAL_ItemInstance))
		{
			//todo::在操作不同的库存组件时，如果将目标物品从其他库存组件拖拽我自身的库存组件中那么就需要额外操作
			UInvSys_InventoryComponent* From_InvComp = LOCAL_ItemInstance->GetInventoryComponent();
			if (InventoryComponent == From_InvComp)
			{
				PlayerInvComp->Server_CancelDragItemInstance(From_InvComp);
				PlayerInvComp->Server_EquipItemInstance(From_InvComp, LOCAL_ItemInstance, SlotTag);
			}
			else
			{
				//todo::这里逻辑应该有问题，DraggingItem的值未被处理
				// auto ItemDefinition = LOCAL_ItemInstance->GetItemDefinition();
				// From_InvComp->UnEquipItemInstance(LOCAL_ItemInstance);
				// InventoryComponent->EquipItemDefinition(ItemDefinition, SlotTag);
			}
			return true;
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

bool UInvSys_EquipSlotWidget::CheckIsCanDrop_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	return true;
}
