// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_EquipItem.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_EquipItem : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	FGameplayTagContainer SupportEquipSlot;

	// 是否允许玩家更改
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	bool bIsAllowPlayerChange = true;
};
