// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Widgets/Components/InvSys_DragDropOperation.h"
#include "InvSys_ItemFragment_DragDrop.generated.h"

class AInvSys_PickableItems;
class UInvSys_DraggingItemWidget;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_ItemFragment_DragDrop : public UInvSys_InventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment|Drop")
	TSubclassOf<AInvSys_PickableItems> DropItemClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment|Drop")
	TSubclassOf<UDragDropOperation> DragDropOperationClass = UInvSys_DragDropOperation::StaticClass();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment|Drop")
	FVector DropLocationOffset;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment|Drop")
	TSoftObjectPtr<UStaticMesh> DropDisplayMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory Item Fragment|Drag")
	TSubclassOf<UInvSys_DraggingItemWidget> DraggingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment|Drag")
	EDragPivot DragPivot = EDragPivot::CenterCenter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment|Drag")
	FVector2D DragOffset = FVector2D(0, 0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item Fragment|Drag")
	bool bIsAutoRotation = true;
};
