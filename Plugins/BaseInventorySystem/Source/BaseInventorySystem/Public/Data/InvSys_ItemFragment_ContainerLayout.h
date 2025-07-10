// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_ContainerLayout.generated.h"

class UInvSys_InventoryWidget;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_ContainerLayout : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TSubclassOf<UInvSys_InventoryWidget> ContainerLayout;
};
