// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryItemInfo.h"
#include "GridInvSys_InventoryContainerInfo.generated.h"

class UGridInvSys_ContainerGridLayoutWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryContainerInfo : public UGridInvSys_InventoryItemInfo
{
	GENERATED_BODY()

protected:
	UFUNCTION(CallInEditor)
	void GetAllContainerGridSize();
	
public:
	// 布局控件
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Container Info")
	TSubclassOf<UGridInvSys_ContainerGridLayoutWidget> ContainerGridLayoutWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Container Info")
	TMap<FName, FIntPoint> ContainerGridSizeMap;
};
