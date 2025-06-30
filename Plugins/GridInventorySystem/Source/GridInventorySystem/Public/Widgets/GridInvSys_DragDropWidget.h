// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_DragDropWidget.generated.h"

class UInvSys_InventoryItemInfo;
enum class EDragPivot : uint8;
class UGridInvSys_DragItemWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_DragDropWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UGridInvSys_DragDropWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo);

	void SetDraggingWidgetClass(TSubclassOf<UGridInvSys_DragItemWidget> NewDraggingWidgetClass);
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	TObjectPtr<UInvSys_InventoryItemInfo> ItemInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop Widget")
	bool bIsOverrideDraggingWidgetClass = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget", meta = (EditCondition = "OverrideDraggingWidgetClass"))
	TSubclassOf<UGridInvSys_DragItemWidget> DraggingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget")
	EDragPivot DragPivot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget")
	FVector2D DragOffset;
};
