// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_ContainerPriority.generated.h"

/**
 * 容器优先级，决定该物品在拾取时优先加入的容器
 * todo::无该片段的物品无法加入容器?
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_ContainerPriority : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TArray<FGameplayTag> ContainerPriority;
};
