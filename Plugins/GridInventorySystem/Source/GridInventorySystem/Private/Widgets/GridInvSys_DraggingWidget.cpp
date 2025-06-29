// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DraggingWidget.h"

void UGridInvSys_DraggingWidget::UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo)
{
	ItemInfo = NewItemInfo;

	OnUpdateItemInfo();
}
