// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_PickUpItem.generated.h"

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_PickUpItem : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TSubclassOf<AActor> PickUpActor;

	/** 拾取物品时优先放置的容器顺序 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TArray<FGameplayTag> ContainerPriority;

	/** 拾取的物品禁止放置的容器 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	FGameplayTagContainer BlockContainer;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	bool bAllowStack = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	int32 MaxStackCount = 1;
};
