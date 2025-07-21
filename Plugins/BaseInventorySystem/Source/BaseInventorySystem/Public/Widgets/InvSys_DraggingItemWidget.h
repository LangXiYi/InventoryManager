// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSys_DraggingItemWidget.generated.h"

class UInvSys_InventoryItemInstance;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_DraggingItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn))
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;
};
