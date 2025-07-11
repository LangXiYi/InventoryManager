// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipmentSlotWidget.h"

#include "InvSys_CommonType.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/GridInvSys_ItemFragment_ItemType.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
#include "Widgets/GridInvSys_DragDropWidget.h"


bool UGridInvSys_EquipmentSlotWidget::CheckIsCanDrop_Implementation(UInvSys_InventoryItemInstance* InItemInstance)
{
	auto ItemTypeFragment = InItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_ItemType>();
	if (ItemTypeFragment)
	{
		return EquipItemType == ItemTypeFragment->ItemType;
	}
	return Super::CheckIsCanDrop_Implementation(InItemInstance);
}
