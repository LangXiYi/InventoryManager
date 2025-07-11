// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GridInvSys_ItemFragment_ItemType.generated.h"

enum class EGridInvSys_InventoryItemType : uint8;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ItemFragment_ItemType : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	EGridInvSys_InventoryItemType ItemType = EGridInvSys_InventoryItemType::None;
};
