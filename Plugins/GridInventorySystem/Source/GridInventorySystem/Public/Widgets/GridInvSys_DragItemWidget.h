// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_DragItemWidget.generated.h"

class UGridInvSys_ContainerGridItemWidget;
class UGridInvSys_DragDropWidget;
class UInvSys_InventoryItemInstance;
class UImage;
class UGridInvSys_InventoryItemInfo;
class USizeBox;
class UInvSys_InventoryItemInfo;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_DragItemWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

	friend UGridInvSys_DragDropWidget;
	friend UGridInvSys_ContainerGridItemWidget;

public:
	void SetDirection(EGridInvSys_ItemDirection NewDirection);

private:
	void SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dragging Widget")
	EGridInvSys_ItemDirection ItemDirection = EGridInvSys_ItemDirection::Horizontal;

	UPROPERTY(BlueprintReadOnly, Category = "Dragging Widget")
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Item Widget", meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_DragItem;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Item Widget", meta = (BindWidget))
	TObjectPtr<UImage> Image_DragItem;
};
