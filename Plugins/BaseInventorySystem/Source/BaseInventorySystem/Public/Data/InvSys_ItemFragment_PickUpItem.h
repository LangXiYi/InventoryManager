// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_PickUpItem.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_PickUpItem : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TSubclassOf<AActor> PickUpActor;
};
