// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InvSys_InventoryContentMapping.generated.h"

class UInvSys_InventoryLayoutWidget;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryContentMapping : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TSubclassOf<UInvSys_InventoryLayoutWidget> InventoryLayout;
	
	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TArray<class UInvSys_PreEditInventoryObject*> InventoryContentList;

};
