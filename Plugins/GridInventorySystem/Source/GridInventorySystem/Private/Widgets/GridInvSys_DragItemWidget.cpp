// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragItemWidget.h"

#include "BaseInventorySystem.h"
#include "GridInvSys_InventorySystemConfig.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Data/GridInvSys_InventoryItemInfo.h"

void UGridInvSys_DragItemWidget::UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo)
{
	GridItemInfo = Cast<UGridInvSys_InventoryItemInfo>(NewItemInfo);
	if (const UGridInvSys_InventorySystemConfig* SystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>())
	{
		SizeBox_DragItem->SetHeightOverride(GridItemInfo->ItemSize.X * SystemConfig->ItemDrawSize);
		SizeBox_DragItem->SetWidthOverride(GridItemInfo->ItemSize.Y * SystemConfig->ItemDrawSize);
	}
	ItemSize = GridItemInfo->ItemSize;
	Image_DragItem->SetBrushFromTexture(GridItemInfo->ItemImage);
}

void UGridInvSys_DragItemWidget::UpdateDirection(EGridInvSys_ItemDirection NewDirection)
{
	ItemDirection = NewDirection;
	switch (NewDirection)
	{
	case EGridInvSys_ItemDirection::Horizontal:
		ItemSize = GridItemInfo->ItemSize;
		SetRenderTransformAngle(0);
		break;
	case EGridInvSys_ItemDirection::Vertical:
		ItemSize = FIntPoint(GridItemInfo->ItemSize.Y, GridItemInfo->ItemSize.X);
		SetRenderTransformAngle(90);
		break;
	}
}
