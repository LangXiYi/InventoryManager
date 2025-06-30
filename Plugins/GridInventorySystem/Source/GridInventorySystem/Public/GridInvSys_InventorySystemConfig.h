// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventorySystemConfig.h"
#include "GridInvSys_InventorySystemConfig.generated.h"

class UGridInvSys_DragItemWidget;
/**
 * 
 */
UCLASS(DefaultConfig, Config = "GridInventorySystemConfig")
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventorySystemConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// UDeveloperSettings Begin =====
	virtual FName GetContainerName() const override { return "Project"; }
	virtual FName GetCategoryName() const override { return "InventorySystem"; }
	virtual FName GetSectionName() const override { return "InventorySystemConfig"; }

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Inventory System Config")
	TSubclassOf<UGridInvSys_DragItemWidget> DraggingWidgetClass = nullptr;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Inventory System Config")
	int32 ItemDrawSize = 128;
};
