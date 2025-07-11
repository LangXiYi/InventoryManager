// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragItemWidget.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_EquippedIcon.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"

/*void UGridInvSys_DragItemWidget::UpdateItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (NewItemInstance)
	{
		ItemInstance = NewItemInstance;
		auto GridItemSizeFragment = ItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>();
		auto ImageFragment = ItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_EquippedIcon>();
		auto SystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>();
		if (GridItemSizeFragment && ImageFragment && SystemConfig)
		{
			SizeBox_DragItem->SetHeightOverride(GridItemSizeFragment->ItemSize.X * SystemConfig->ItemDrawSize);
			SizeBox_DragItem->SetWidthOverride(GridItemSizeFragment->ItemSize.Y * SystemConfig->ItemDrawSize);

			Image_DragItem->SetBrushFromTexture(ImageFragment->EquippedIcon);
		}
	}
}

void UGridInvSys_DragItemWidget::UpdateItemDirection(EGridInvSys_ItemDirection NewDirection)
{
	ItemDirection = NewDirection;
}*/