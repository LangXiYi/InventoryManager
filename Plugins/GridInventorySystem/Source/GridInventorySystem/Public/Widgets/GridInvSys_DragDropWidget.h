// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_DragDropWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;

UENUM()
enum class EDragDropType : uint8
{
	None = 0,
	Container,
	Equipment,
};

class UInvSys_InventoryComponent;
class UInvSys_InventoryItemInfo;
enum class EDragPivot : uint8;
class UGridInvSys_DragItemWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_DragDropWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	
	template<class T>
	T* GetItemInfo() const
	{
		return Cast<T>(nullptr);		
	}
	
	template<class T>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance;
	}

	UGridInvSys_ContainerGridItemWidget* GetGridItemWidget() const;

	EDragDropType GetDragDropType() const;

	void SetDragDropType(EDragDropType NewDragType);

	void SetDirection(EGridInvSys_ItemDirection NewDirection)
	{
		
	}
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	//void SetDragDropType(EDragDropType NewDragType);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	UInvSys_InventoryItemInstance* ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Drag Drop Widget")
	EDragDropType DragDropType = EDragDropType::Container;
};
