// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventorySystemConfig.h"
#include "GridInvSys_InventorySystemConfig.generated.h"

class UGridInvSys_DraggingWidget;
/**
 * 
 */
UCLASS(DefaultConfig, Config = "InventorySystemConfig")
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventorySystemConfig : public UInvSys_InventorySystemConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Inventory System Config")
	TSubclassOf<UGridInvSys_DraggingWidget> DraggingWidgetClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Inventory System Config")
	int32 ItemDrawSize = 128;
};
