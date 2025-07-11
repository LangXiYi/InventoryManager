// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "InvSys_ItemFragment_DragDrop.generated.h"

class UInvSys_DraggingItemInterface;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_DragDrop : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TSubclassOf<UUserWidget> DraggingWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TScriptInterface<UInvSys_DraggingItemInterface> DraggingItemWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	EDragPivot DragPivot = EDragPivot::CenterCenter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	FVector2D DragOffset = FVector2D(0, 0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	bool bIsAutoRotation = true;
};
