// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragItemWidget.h"


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