// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_DragDropWidget.generated.h"

class UGridInvSys_ContainerGridItemWidget;

UENUM()
enum class EDragDropType : uint8
{
	None = 0,
	Container,
	Equipment,
};

static TMap<EDragDropType, FString> DragDropTypeString = {
	{EDragDropType::None, "None"},
	{EDragDropType::Container, "Container"},
	{EDragDropType::Equipment, "Equipment"},
};
static TMap<FString, EDragDropType> DragDropTypeEnum = {
	{"None", EDragDropType::None},
	{"Container", EDragDropType::Container},
	{"Equipment", EDragDropType::Equipment},
};

class UInvSys_InventoryComponent;
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

	void SetDraggingWidgetClass(TSubclassOf<UGridInvSys_DragItemWidget> NewDraggingWidgetClass);
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

public:
	template<class T>
	T* GetItemInfo() const
	{
		return Cast<T>(ItemInfo);		
	}
	
	UFUNCTION(BlueprintCallable)
	void UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo);

	void SetGridItemWidget(UGridInvSys_ContainerGridItemWidget* NewGridItemWidget);

	UGridInvSys_ContainerGridItemWidget* GetGridItemWidget() const;

	EDragDropType GetDragDropType() const;

	void SetDragDropType(EDragDropType NewDragType);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	TObjectPtr<UInvSys_InventoryItemInfo> ItemInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	TObjectPtr<UGridInvSys_ContainerGridItemWidget> GridItemWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	EDragDropType DragDropType = EDragDropType::Container;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop Widget")
	bool bIsOverrideDraggingWidgetClass = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget", meta = (EditCondition = "OverrideDraggingWidgetClass"))
	TSubclassOf<UGridInvSys_DragItemWidget> DraggingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget")
	EDragPivot DragPivot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop Widget")
	FVector2D DragOffset;
};
