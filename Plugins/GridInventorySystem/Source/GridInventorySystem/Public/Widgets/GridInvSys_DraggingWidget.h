// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_DraggingWidget.generated.h"

class UInvSys_InventoryItemInfo;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_DraggingWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Dragging Widget")
	void OnUpdateItemInfo();

protected:
	TObjectPtr<UInvSys_InventoryItemInfo> ItemInfo;
};
