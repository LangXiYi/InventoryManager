// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GridInvSys_ItemFragment_GridItemSize.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ItemFragment_GridItemSize : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	FIntPoint ItemSize = FIntPoint(1, 1);
};
