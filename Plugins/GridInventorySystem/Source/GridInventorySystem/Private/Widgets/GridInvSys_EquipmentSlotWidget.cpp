// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipmentSlotWidget.h"

#include "InvSys_CommonType.h"
#include "Widgets/GridInvSys_DragDropWidget.h"


void UGridInvSys_EquipmentSlotWidget::UpdateOccupant(const FInvSys_InventoryItem& NewOccupant)
{
	DragDropWidget->UpdateItemInfo(NewOccupant.ItemInfo);
	OnUpdateOccupant(NewOccupant);
}

void UGridInvSys_EquipmentSlotWidget::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	Super::SetInventoryComponent(NewInvComp);
	DragDropWidget->SetInventoryComponent(NewInvComp);
}
