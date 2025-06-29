// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InvSys_InventoryContentMapping.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryContentMapping : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TArray<class UInvSys_PreEditInventoryObject*> InventoryContentList;

};
