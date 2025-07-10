// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GridInvSys_ItemFragment_EquippedIcon.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ItemFragment_EquippedIcon : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TObjectPtr<UTexture2D> EquippedIcon;
};
