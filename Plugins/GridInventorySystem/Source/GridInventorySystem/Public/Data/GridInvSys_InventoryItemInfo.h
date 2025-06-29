// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Data/InvSys_InventoryItemInfo.h"
#include "GridInvSys_InventoryItemInfo.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryItemInfo : public UInvSys_InventoryItemInfo
{
	GENERATED_BODY()

public:
	// 物品大小
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Inventory Item Info")
	FIntPoint ItemSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Inventory Item Info")
	EGridInvSys_InventoryItemType ItemType = EGridInvSys_InventoryItemType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Inventory Item Info")
	TObjectPtr<UTexture2D> ItemImage;
};
