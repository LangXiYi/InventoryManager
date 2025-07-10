// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipContainerSlotWidget.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "GridInvSys_CommonType.h"


UGridInvSys_ContainerGridItemWidget* UGridInvSys_EquipContainerSlotWidget::FindGridItemWidget(
	const FGridInvSys_ItemPosition& ItemPosition) const
{
	if (GetSlotTag() != ItemPosition.EquipSlotTag)
	{
		return nullptr;
	}
	if (ContainerLayoutWidget && ContainerLayoutWidget->IsA(UGridInvSys_ContainerGridLayoutWidget::StaticClass()))
	{
		UGridInvSys_ContainerGridLayoutWidget* LayoutWidget = Cast<UGridInvSys_ContainerGridLayoutWidget>(ContainerLayoutWidget);
		check(LayoutWidget);
		return LayoutWidget->FindGridItemWidget(ItemPosition);
	}
	return nullptr;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_EquipContainerSlotWidget::FindGridItemWidget(
	const UInvSys_InventoryItemInstance* NewItemInstance) const
{
	if (NewItemInstance && NewItemInstance->IsA(UGridInvSys_InventoryItemInstance::StaticClass()))
	{
		UGridInvSys_InventoryItemInstance* TempItemInstance = (UGridInvSys_InventoryItemInstance*)NewItemInstance;
		check(TempItemInstance);
		return FindGridItemWidget(TempItemInstance->GetItemPosition());
	}
	return nullptr;
}
