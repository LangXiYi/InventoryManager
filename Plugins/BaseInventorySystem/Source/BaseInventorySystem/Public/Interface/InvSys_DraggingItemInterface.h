// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InvSys_DraggingItemInterface.generated.h"

class UInvSys_InventoryItemInstance;

UINTERFACE()
class UInvSys_DraggingItemInterface : public UInterface
{
	GENERATED_BODY()
};

class BASEINVENTORYSYSTEM_API IInvSys_DraggingItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Dragging Item Interface")
	void UpdateItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dragging Item Interface")
	void OnCancelDragItemInstance();
};
