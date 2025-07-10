// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GridInvSys_ItemFragment_DragDrop.generated.h"

class UGridInvSys_DragItemWidget;
class UGridInvSys_DragDropWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ItemFragment_DragDrop : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment")
	TSubclassOf<UGridInvSys_DragItemWidget> DraggingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	EDragPivot DragPivot = EDragPivot::CenterCenter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	FVector2D DragOffset = FVector2D(0, 0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment")
	bool bIsAutoRotation = true;
};
