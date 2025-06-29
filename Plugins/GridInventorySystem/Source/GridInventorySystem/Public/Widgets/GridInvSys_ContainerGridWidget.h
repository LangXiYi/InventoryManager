// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridWidget.generated.h"

struct FGridInvSys_InventoryItem;
class UGridInvSys_ContainerGridDraggingDisplayWidget;
class UGridInvSys_ContainerGridItemWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void ConstructGridItems(FName NewSlotName);

	void AddInventoryItemTo(const FGridInvSys_InventoryItem& InventoryItem);

public:
	bool HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UGridInvSys_ContainerGridItemWidget*>& Ignores = {});

	void GetInventoryItemsByAre(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, FIntPoint Position, FIntPoint Size);

	void FindContainerGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, FIntPoint Position,
		FIntPoint ItemSize, const TSet<UGridInvSys_ContainerGridItemWidget*>& Ignores = {});

	UGridInvSys_ContainerGridItemWidget* GetContainerGridItem(FIntPoint Position);

	FORCEINLINE FName GetContainerGridID() const;
	FORCEINLINE void SetContainerGridID(FName NewContainerGridID);

	FORCEINLINE FIntPoint GetContainerGridSize()
	{
		return ContainerGridSize;
	}

protected:
	virtual void NativeConstruct() override;

	virtual void NativePreConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UGridPanel> ContainerGridPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> DragDisplayGridPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_ContainerGridItemWidget> GridItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_ContainerGridDraggingDisplayWidget> GridDraggingDisplayWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FIntPoint ContainerGridSize = FIntPoint(1, 1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FName ContainerGridID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FName SlotName;
};
