// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipmentSlotWidget.h"


void UGridInvSys_EquipmentSlotWidget::UpdateOccupant(const FInvSys_InventoryItem& NewOccupant)
{
	
	OnUpdateOccupant(NewOccupant);
}
