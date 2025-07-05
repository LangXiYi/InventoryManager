// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_DragItemWidget.generated.h"

class UImage;
class UGridInvSys_InventoryItemInfo;
class USizeBox;
class UInvSys_InventoryItemInfo;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_DragItemWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo);

	void UpdateDirection(EGridInvSys_ItemDirection NewDirection);

	EGridInvSys_ItemDirection GetDirection() const
	{
		return ItemDirection;
	}

	FIntPoint GetItemSize() const
	{
		return  ItemSize;
	}
	
protected:
	EGridInvSys_ItemDirection ItemDirection;

	FIntPoint ItemSize;
	
	UPROPERTY(BlueprintReadOnly, Category = "Dragging Widget")
	TObjectPtr<UGridInvSys_InventoryItemInfo> GridItemInfo;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Item Widget", meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_DragItem;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Item Widget", meta = (BindWidget))
	TObjectPtr<UImage> Image_DragItem;
};
