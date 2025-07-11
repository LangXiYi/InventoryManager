// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "UObject/Interface.h"
#include "GridInvSys_DraggingItemInterface.generated.h"

UINTERFACE()
class UGridInvSys_DraggingItemInterface : public UInterface
{
	GENERATED_BODY()
};

class GRIDINVENTORYSYSTEM_API IGridInvSys_DraggingItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Dragging Item Interface")
	void UpdateItemDirection(EGridInvSys_ItemDirection NewDirection);
};
